#include <print>
#include <SDL.h>

#include "CPU/cpu.h"

struct SDLGuard {
    SDLGuard() = default;

    ~SDLGuard() {
        SDL_Quit();
    }

    SDLGuard(const SDLGuard &) = delete;

    SDLGuard &operator=(const SDLGuard &) = delete;
};

struct SDLDeleter {
    void operator()(SDL_Window *window) const {
        SDL_DestroyWindow(window);
    }

    void operator()(SDL_Renderer *renderer) const {
        SDL_DestroyRenderer(renderer);
    }

    void operator()(SDL_Texture *texture) const {
        SDL_DestroyTexture(texture);
    }
};

int main(int argc, char *argv[]) {
    std::println("BioGB v5.0");
    if (argc < 2) {
        std::println(stderr, "Usage: biogb <game.gb>");
        return EXIT_FAILURE;
    }
    const std::string rom_file_name{argv[1]};
    cCpu gb;
    if (!gb.init_cpu(rom_file_name)) {
        std::println(stderr, "Initialization failed: {}", rom_file_name);
        return EXIT_FAILURE;
    }

    constexpr int screen_width{160}, screen_height{144};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::println(stderr, "ERROR: Could not start SDL: {}", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDLGuard guard; // This is used for calling SDL_Quit automatically.

    std::println("Starting SDL Video");
    // When the pointer goes out of scope
    // 1. unique_ptr checks if the pointer is not null
    // 2. Creates an instance of SDLDeleter
    // 3. Calls SDLDeleter::operator()(SDL_Window*)
    // 4. Which calls SDL_DestroyWindow(window)

    std::unique_ptr<SDL_Window, SDLDeleter> screen{
        SDL_CreateWindow(
            "BioGB v5.0",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            screen_width * 3,
            screen_height * 3,
            0
        )
    };

    if (!screen) {
        std::println(stderr, "ERROR: Could not create window: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    std::unique_ptr<SDL_Renderer, SDLDeleter> renderer{
        SDL_CreateRenderer(
            screen.get(), -1, 0)
    };
    std::unique_ptr<SDL_Texture, SDLDeleter> texture{
        SDL_CreateTexture(
            renderer.get(),
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            screen_width,
            screen_height
        )
    };
    constexpr SDL_Rect window_size = {0, 0, screen_width * 3, screen_height * 3};

    std::println("Starting SDL Audio");
    cSound *sound = gb.get_sound_system();
    SDL_AudioSpec request{};

    constexpr int sample_rate = 44100;
    constexpr int buffer_size = 512;
    constexpr int channels = 2;

    request.freq = sample_rate;
    request.format = AUDIO_U8;
    request.channels = static_cast<Uint8>(channels);
    request.samples = static_cast<Uint16>(buffer_size);
    request.callback = [](void *data, u8 *buffer, const s32 size) noexcept {
        static_cast<cSound *>(data)->fillBuffer(buffer, size);
    };
    request.userdata = sound;

    SDL_AudioSpec result;
    const SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(nullptr, 0, &request, &result, 0);
    if (device_id == 0) {
        std::println(stderr, "ERROR: Could not open audio device: {}", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_PauseAudioDevice(device_id, 0);
    constexpr int pitch = screen_width * sizeof(unsigned int);

    Uint64 time_to_next_frame;
#if SDL_VERSION_ATLEAST(2, 0, 18)
    const auto now = SDL_GetTicks64();
    time_to_next_frame = now + 16;
#else
    time_to_next_frame = SDL_GetTicks() + 16u;
#endif


    int fps = 0;
    Uint64 fps_time = SDL_GetTicks64() + 1000;
    Uint64 frame_start_time = SDL_GetTicks64();
    Uint64 frame_count = 0;
    std::string title;
    double fpsSpeed = 1.0;
    bool is_running = true;

    SDL_Event event;
    while (is_running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    is_running = false;
                    // gb.save_sram();
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_s:
                            std::println("Saving state");
                            break;
                        case SDLK_d:
                            fpsSpeed -= 0.5;
                            fpsSpeed = std::clamp(fpsSpeed, 0.5, 3.0);
                            frame_start_time = SDL_GetTicks64();
                            frame_count = 0;
                            break;
                        case SDLK_a:
                            fpsSpeed += 0.5;
                            fpsSpeed = std::clamp(fpsSpeed, 0.5, 3.0);
                            frame_start_time = SDL_GetTicks64();
                            frame_count = 0;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        auto key_state = SDL_GetKeyboardState(nullptr);
        gb.reset_input();


        if (key_state[SDL_SCANCODE_ESCAPE]) {
            is_running = false;
        }
        if (key_state[SDL_SCANCODE_Z]) {
            gb.update_input(GBKey::A);
        }
        if (key_state[SDL_SCANCODE_X]) {
            gb.update_input(GBKey::B);
        }
        if (key_state[SDL_SCANCODE_RETURN]) {
            gb.update_input(GBKey::Start);
        }
        if (key_state[SDL_SCANCODE_RSHIFT]) {
            gb.update_input(GBKey::Select);
        }
        if (key_state[SDL_SCANCODE_UP]) {
            gb.update_input(GBKey::Up);
        }
        if (key_state[SDL_SCANCODE_DOWN]) {
            gb.update_input(GBKey::Down);
        }
        if (key_state[SDL_SCANCODE_LEFT]) {
            gb.update_input(GBKey::Left);
        }
        if (key_state[SDL_SCANCODE_RIGHT]) {
            gb.update_input(GBKey::Right);
        }
        if (!key_state[SDL_SCANCODE_SPACE]) {
            frame_count++;
            const Uint64 target_time = frame_start_time + frame_count / fpsSpeed * 1000 / 60;
            if (const auto current_time = SDL_GetTicks64(); target_time > current_time) {
                SDL_Delay(target_time - current_time);
            }
        }
        fps++;
        if (fps_time <= SDL_GetTicks64()) {
            title = "BioGB v5.0 - FPS: " + std::to_string(fps);
            SDL_SetWindowTitle(screen.get(), title.c_str());
            fps_time = SDL_GetTicks64() + 1000;
            fps = 0;
        }
        gb.runFrame();
        auto video_buffer = gb.get_video_buffer();
        SDL_UpdateTexture(texture.get(), nullptr, video_buffer.data(), pitch);
        SDL_RenderCopy(renderer.get(), texture.get(), nullptr, &window_size);
        SDL_RenderPresent(renderer.get());
    }
    SDL_PauseAudioDevice(device_id, 1);

    SDL_CloseAudioDevice(device_id);
    return EXIT_SUCCESS;
}

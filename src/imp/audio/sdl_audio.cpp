#include "sdl_audio.h"

#include <iostream>

void outputAudio(void *data, u8 *a_internalBuffer, s32 a_bufferSize)
{
    reinterpret_cast<cSDLSound *>(data)->fillBuffer(a_internalBuffer, a_bufferSize);
}

cSDLSound::cSDLSound(int a_generalFrequency, int a_bufferSize) : cSound(a_generalFrequency)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        throw std::runtime_error("Can't init audio system: " + std::string(SDL_GetError()) + "\n");
    SDL_AudioSpec request;
    SDL_memset(&request, 0, sizeof(request));

    request.freq = a_generalFrequency;
    request.format = AUDIO_U8;
    request.channels = static_cast<Uint8>(NUMBER_OF_CHANNELS);
    request.samples = static_cast<Uint16>(a_bufferSize);
    request.callback = outputAudio;
    request.userdata = this;

    SDL_AudioSpec result;
    mDevice = SDL_OpenAudioDevice(nullptr, 0, &request, &result, 0);
    if (mDevice == 0)
        throw std::runtime_error("Can't open audio device: " + std::string(SDL_GetError()) + "\n");
}

cSDLSound::~cSDLSound()
{
    SDL_CloseAudioDevice(mDevice);
}

void cSDLSound::turnOn()
{
    SDL_PauseAudioDevice(mDevice, 0);
}

void cSDLSound::turnOff()
{
    SDL_PauseAudioDevice(mDevice, 1);
}


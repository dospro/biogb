#include "sdl_display.h"

cSDLDisplay::cSDLDisplay(bool a_isColor)
    : mScreen{nullptr},
      mRenderer{nullptr},
      mTexture{nullptr},
      cDisplay(a_isColor) {
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("SDL: Couldn't init SDL Video: %s\n", SDL_GetError());
            return;
        }
    }

    mScreen = SDL_CreateWindow("BioGB v5.0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mScreenWidth * 3,
                               mScreenHeight * 3, 0);
    if (!mScreen) {
        puts("SDL: Error creating screen");
        return;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    mRenderer = SDL_CreateRenderer(mScreen, -1, 0);
    mTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, mScreenWidth,
                                 mScreenHeight);
    mWindowSize = {0, 0, mScreenWidth * 3, mScreenHeight * 3};
}

cSDLDisplay::~cSDLDisplay() {
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
    }
    if (mRenderer != nullptr) {
        SDL_DestroyRenderer(mRenderer);
    }
    if (mScreen != nullptr) {
        SDL_DestroyWindow(mScreen);
    }
    if (SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) SDL_Quit();
}

void cSDLDisplay::updateScreen() {
    SDL_UpdateTexture(mTexture, nullptr, videoBuffer, mScreenWidth * sizeof(unsigned int));
    SDL_RenderCopy(mRenderer, mTexture, nullptr, &mWindowSize);
    SDL_RenderPresent(mRenderer);
}

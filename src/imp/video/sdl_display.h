#ifndef BIOGB_DISPLAY
#define BIOGB_DISPLAY

#include <SDL.h>

#include "../../display.h"

class cSDLDisplay : public cDisplay {
   public:
    cSDLDisplay(bool a_isColor);
    ~cSDLDisplay();
    virtual void updateScreen(void);

   private:
    SDL_Window *mScreen;
    SDL_Renderer *mRenderer;
    SDL_Texture *mTexture;
    SDL_Rect mWindowSize;
};

#endif

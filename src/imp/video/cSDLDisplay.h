/*
 *     Proyect: BioGB
 *    Filename: cSDLGfx.h
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rubén Daniel Gutiérez Cruz <dospro@gmail.com>
 *        Date: 07-1-2007
 *
 *
 *	This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef BIOGB_DISPLAY
#define BIOGB_DISPLAY

#include<stdlib.h>
#include<SDL.h>
#include"../../cDisplay.h"


class cSDLDisplay : public cDisplay
{
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

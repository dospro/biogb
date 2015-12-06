/*
 *     Proyect: BioGB
 *    Filename: cSDLGfx.cpp
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rub� Daniel Guti�rez Cruz <dospro@gmail.com>
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


#include"cSDLDisplay.h"

#define XRES 160*3
#define YRES 144*3

cSDLDisplay::cSDLDisplay(bool a_isColor) :
        screen{nullptr},
        back{nullptr},
        cDisplay(a_isColor)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            printf("SDL: Couldn't init SDL Video: %s\n", SDL_GetError());
            return;
        }
    }

    screen = SDL_SetVideoMode(XRES, YRES, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        puts("SDL: Error creating screen");
        return;
    }

    back = SDL_CreateRGBSurface(SDL_SWSURFACE, XRES, YRES, 32, 0xFF0000, 0xFF00, 0xFF, 0);
    if (back == NULL)
    {
        puts("SDL: Error creating backbuffer");
        return;
    }
}

cSDLDisplay::~cSDLDisplay()
{
    if (back != NULL)
    {
        SDL_FreeSurface(back);
        back = NULL;
    }
    if (SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO)
        SDL_Quit();
}

void cSDLDisplay::putPixel(int x, int y, u32 color)
{
    u8 *p = (u8 *) back->pixels + y * back->pitch + x * 4;
    *(Uint32 *) p = color;
}

void cSDLDisplay::updateScreen(void)
{
    u8 *p;
    int x, y;
    int pixel;
    SDL_LockSurface(back);

    for (x = 0; x < 160; ++x)
    {
        for (y = 0; y < 144; ++y)
        {
            pixel = videoBuffer[x][y];

            putPixel(x * 3, y * 3, pixel);
            putPixel(x * 3 + 1, y * 3, pixel);
            putPixel(x * 3 + 2, y * 3, pixel);
            putPixel(x * 3, y * 3 + 1, pixel);
            putPixel(x * 3 + 1, y * 3 + 1, pixel);
            putPixel(x * 3 + 2, y * 3 + 1, pixel);
            putPixel(x * 3, y * 3 + 2, pixel);
            putPixel(x * 3 + 1, y * 3 + 2, pixel);
            putPixel(x * 3 + 2, y * 3 + 2, pixel);

        }
    }

    SDL_UnlockSurface(back);

    SDL_BlitSurface(back, NULL, screen, NULL);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

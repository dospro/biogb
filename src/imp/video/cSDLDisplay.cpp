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

cSDLDisplay::cSDLDisplay()
{
        screen = NULL;
        back = NULL;
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

bool cSDLDisplay::init(void)
{

        if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        {
                if (SDL_Init(SDL_INIT_VIDEO) != 0)
                {
                        printf("SDL: Couldn't init SDL Video: %s\n", SDL_GetError());
                        return false;
                }
        }

        screen = SDL_SetVideoMode(XRES, YRES, 32, SDL_SWSURFACE);
        if (screen == NULL)
        {
                puts("SDL: Error creating screen");
                return false;
        }

        back = SDL_CreateRGBSurface(SDL_SWSURFACE, XRES, YRES, 32, 0xFF0000, 0xFF00, 0xFF, 0);
        //back=SDL_CreateRGBSurfaceFrom(videoBuffer, 160, 144, 32, 144*4, 0xFF0000, 0xFF00, 0xFF, 0);
        if (back == NULL)
        {
                puts("SDL: Error creating backbuffer");
                return false;
        }

        return cDisplay::init();
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
        int A, B, C, D, E, F, G, H, I;
        int E0, E1, E2, E3, E4, E5, E6, E7, E8;
        SDL_LockSurface(back);

        for (x = 0; x < 160; ++x)
        {
                for (y = 0; y < 144; ++y)
                {
                        if (x >= 158)
                        {
                                if (y >= 142)
                                {
                                        A = videoBuffer[x][y];
                                        B = videoBuffer[x][y];
                                        C = videoBuffer[x][y];
                                        D = videoBuffer[x][y];
                                        E = videoBuffer[x][y];
                                        F = videoBuffer[x][y];
                                        G = videoBuffer[x][y];
                                        H = videoBuffer[x][y];
                                        I = videoBuffer[x][y];
                                }
                                else
                                {
                                        A = videoBuffer[x][y];
                                        B = videoBuffer[x][y];
                                        C = videoBuffer[x][y];
                                        D = videoBuffer[x][y + 1];
                                        E = videoBuffer[x][y + 1];
                                        F = videoBuffer[x][y + 1];
                                        G = videoBuffer[x][y + 2];
                                        H = videoBuffer[x][y + 2];
                                        I = videoBuffer[x][y + 2];
                                }
                        }
                        else
                        {
                                if (y >= 142)
                                {
                                        A = videoBuffer[x][y];
                                        B = videoBuffer[x + 1][y];
                                        C = videoBuffer[x + 2][y];
                                        D = videoBuffer[x][y];
                                        E = videoBuffer[x + 1][y];
                                        F = videoBuffer[x + 2][y];
                                        G = videoBuffer[x][y];
                                        H = videoBuffer[x + 1][y];
                                        I = videoBuffer[x + 2][y];
                                }
                                else
                                {
                                        A = videoBuffer[x][y];
                                        B = videoBuffer[x + 1][y];
                                        C = videoBuffer[x + 2][y];
                                        D = videoBuffer[x][y + 1];
                                        E = videoBuffer[x + 1][y + 1];
                                        F = videoBuffer[x + 2][y + 1];
                                        G = videoBuffer[x][y + 2];
                                        H = videoBuffer[x + 1][y + 2];
                                        I = videoBuffer[x + 2][y + 2];
                                }
                        }
                        if (B != H && D != F)
                        {
                                E0 = D == B ? D : E;
                                E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
                                E2 = B == F ? F : E;
                                E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
                                E4 = E;
                                E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
                                E6 = D == H ? D : E;
                                E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
                                E8 = H == F ? F : E;
                        }
                        else
                        {
                                E0 = E;
                                E1 = E;
                                E2 = E;
                                E3 = E;
                                E4 = E;
                                E5 = E;
                                E6 = E;
                                E7 = E;
                                E8 = E;
                        }

                        putPixel(x * 3, y * 3, E0);
                        putPixel(x * 3 + 1, y * 3, E1);
                        putPixel(x * 3 + 2, y * 3, E2);
                        putPixel(x * 3, y * 3 + 1, E3);
                        putPixel(x * 3 + 1, y * 3 + 1, E4);
                        putPixel(x * 3 + 2, y * 3 + 1, E5);
                        putPixel(x * 3, y * 3 + 2, E6);
                        putPixel(x * 3 + 1, y * 3 + 2, E7);
                        putPixel(x * 3 + 2, y * 3 + 2, E8);

                }
        }

        SDL_UnlockSurface(back);

        SDL_BlitSurface(back, NULL, screen, NULL);
        SDL_UpdateRect(screen, 0, 0, 0, 0);
}

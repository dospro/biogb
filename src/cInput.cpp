/*
 *     Proyect: BioGB
 *    Filename: cInput.cpp
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rubén Daniel Gutiérrez Cruz <dospro@gmail.com>
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


#include"cInput.h"


bool cInput::initInputSystem(void)
{

    if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
            return false;


    gbky[GB_UP] = GBK_UP;
    gbky[GB_DOWN] = GBK_DOWN;
    gbky[GB_LEFT] = GBK_LEFT;
    gbky[GB_RIGHT] = GBK_RIGHT;
    gbky[GB_A] = GBK_z;
    gbky[GB_B] = GBK_x;
    gbky[GB_START] = GBK_RETURN;
    gbky[GB_SELECT] = GBK_RSHIFT;



    return true;
}

bool cInput::isButtonPressed(int b)
{
    return false;
}

bool cInput::isKeyPressed(int k)
{
    if(event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == k)
        {
            return true;
        }
    }
    return false;
}

bool cInput::isGbKeyPressed(int k)
{
    if (key[gbky[k]])
        return true;
    else
        return false;
}

bool cInput::pollEvents(void)
{
    if (SDL_PollEvent(&event) == 0)
    {
        return false;
    }
    key = (unsigned char *) SDL_GetKeyState(NULL);
    return true;
}

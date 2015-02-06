/*
 *     Proyect: BioGB
 *    Filename: cInput.h
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

#ifndef BIOGB_INPUT
#define BIOGB_INPUT
#include<stdio.h>

#define GBK_ESCAPE SDLK_ESCAPE
#define GBK_SPACE SDLK_SPACE
#define GBK_KP_MINUS SDLK_KP_MINUS
#define GBK_KP_PLUS SDLK_KP_PLUS
#define GBK_s SDLK_s
#define GBK_l SDLK_l
#define GBK_x SDLK_x
#define GBK_z SDLK_z
#define GBK_UP SDLK_UP
#define GBK_DOWN SDLK_DOWN
#define GBK_LEFT SDLK_LEFT
#define GBK_RIGHT SDLK_RIGHT
#define GBK_RETURN SDLK_RETURN
#define GBK_RSHIFT SDLK_RSHIFT



#define GB_UP		0
#define GB_DOWN		1
#define GB_LEFT		2
#define GB_RIGHT	3
#define GB_A		4
#define GB_B		5
#define GB_START	6
#define GB_SELECT	7

class cInput {
protected:

    unsigned char *key; //keyboard

    int gbky[9]; //Gameboy keys
    unsigned int button; //joypad/joystick
public:
    void setKey(int gbKey, int pKey);
    bool initInputSystem(void);
    void pollEvents(void);
    bool isKeyPressed(int k);
    bool isGbKeyPressed(int k);
    bool isButtonPressed(int b);
};

#endif

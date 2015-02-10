/*
 *     Proyect: BioGB
 *    Filename: cSDLGfx.h
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
 
#ifndef BIOGB_LIB_G
#define BIOGB_LIB_GFX

#include<stdlib.h>
#include<SDL.h>
#include"../../cDisplay.h"


class cSDLDisplay: public cDisplay {
	SDL_Surface *screen, *back;

	void putPixel(int x, int y, u32 color);
public:
	cSDLDisplay();
	~cSDLDisplay();
	virtual bool init(void);
	virtual void updateScreen(void);	
};

#endif

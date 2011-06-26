/*
 *     Proyect: BioGB
 *    Filename: cGfx.h
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


#ifndef BIOGB_GFX
#define BIOGB_GFX

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"macros.h"

class cMemory;

struct LCDC {
	bool lcdcActive;
	int wndMap;
	bool wndActive;
	int bgWndData;
	int bgMap;
	int spSize;
	bool spActive;
	bool bgWndActive;
};
struct OAMBlock {
	int x, y;
	int patNum;
	bool priority, xFlip, yFlip, palette, bank;
	int cgbPalete;
};
class cDisplay {
protected:
	cMemory *mem;
	u32 videoBuffer[160][144];
	u32 gbcColors[0x10000];
	/*u32 BWColors[2][2];
	u32 BGPTable[2][2];
	u32 WPTable[2][2];
	u32 OBP0Table[2][2];
	u32 OBP1Table[2][2];
	u32 BGColors[64];
    u32 OBJColors[64];*/
	LCDC lcdc;
	OAMBlock sprite;
	s32 ly;
	u32 prtSym;

	bool dirs[0xFF][0xFF];
	bool lcdcPrt, bgPrt, oamPrt;


	void drawBackGround(void);
	void drawWindow(void);
	void drawSprites(void);
public:
	cDisplay();
	~cDisplay();
	virtual bool init(void);
	virtual void updateScreen(void)=0;
	//void getDisplayMessage(u16 address, u8 value);
	void getMemoryPointer(cMemory *tmem){mem=tmem;};
	void hBlankDraw(void);
};
#endif

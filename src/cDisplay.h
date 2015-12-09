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
#include <array>
#include <vector>
#include"macros.h"

#define BGPI 0xFF68
#define BGPD 0xFF69
#define OBPI 0xFF6A
#define OBPD 0xFF6B


class cMemory;

struct LCDC
{
    bool lcdcActive;
    int wndMap;
    bool wndActive;
    int tileDataAddress;
    int BGMapAddress;
    bool spActive;
    bool bgWndActive;
};

class cDisplay
{
public:
    cDisplay(bool a_isColor);
    virtual ~cDisplay();
    virtual void updateScreen(void) = 0;
    u8 readFromDisplay(u16 a_address);
    void writeToDisplay(u16 a_address, u8 a_value);
    void setVRAMBank(int a_bank);

    void getMemoryPointer(cMemory *tmem)
    { mem = tmem; };
    void hBlankDraw(void);

protected:
    const int TOTAL_OAM_BLOCKS = 40;
    const int SPRITE_ABOVE_BG = 0;
    const int BG_ABOVE_SPRITE = 1;
    const int OAM_SPRITE_ABOVE_BG = 2;
    const int TILE_PATTERN_TABLE_0 = 0; //0x8000 - 0x8000;
    const int TILE_PATTERN_TABLE_1 = 0x0800; //0x8800 - 0x8000;
    const int TILE_MAP_TABLE_0 = 0x1800; // 0x9800 - 0x8000
    const int TILE_MAP_TABLE_1 = 0x1C00; // 0x9C00 - 0x8000
    cMemory *mem;
    std::array<u8, 0x9F> mOAM;
    std::vector<std::array<u8, 0x2000>> mVRAM;
    std::array<u32, 8> mSpriteColorTable;
    std::array<bool, 32> mTilePrioritiesTable;
    int mVRAMBank;
    u32 videoBuffer[160][144];
    u32 gbcColors[0x10000];
    u32 BWColors[2][2];
    u32 BGPTable[2][2];
    u32 WPTable[2][2];
    u32 BGColors[64];
    u32 OBJColors[64];
    LCDC lcdc;
    s32 ly;
    int mFinalPriority;
    int mBGPI;
    int mOBPI;
    bool mIsColor;
    bool mMasterPriority, mOAMPriority;
    void drawBackGround();
    void drawEmptyBG();
    void drawWindow();
    void drawSprites();
    int getPriority();
    void drawSpriteLine(bool flipX, int spriteX, int spritePaletteNumber, u8 firstByte, u8 secondByte);
    bool isSpritePixelVisible(int a_xPosition, int colorIndex, int a_offset) const;
    void setSpriteColorTable(int a_paletteNumber);
    void setBGColorTable(int tileNumber);
    bool isTileVisible(int a_xPosition) const;
    void drawTileLine(int firstByte, int secondByte, int xPosition, bool hFlip);
};

#endif

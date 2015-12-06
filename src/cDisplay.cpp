/*
 *     Proyect: BioGB
 *    Filename: cGfx.cpp
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


#include <iostream>
#include"cDisplay.h"
#include"cCpu.h"

#define COLOR_REDUCTION_FACTOR 1.5
#define COLOR_INFLUENCE_FACTOR 8
#define COLOR_INCREMENT_FACTOR 20


cDisplay::cDisplay(bool a_isColor) :
        BGPTable{{0xFFFFFF, 0xC0C0C0},
                 {0x808080, 0x000000}},
        WPTable{{0xFFFFFF, 0xC0C0C0},
                {0x808080, 0x000000}},
        OBP0Table{{0xFFFFFF, 0xC0C0C0},
                  {0x808080, 0x000000}},
        OBP1Table{{0xFFFFFF, 0xC0C0C0},
                  {0x808080, 0x000000}},
        BWColors{{0xFFFFFF, 0xC0C0C0},
                 {0x808080, 0x000000}},
        mem{nullptr},
        mLCDCPriority{false},
        mIsColor{a_isColor}
{
    int red, green, blue;
    for (int color = 0; color < 0x10000; ++color)
    {
        red = ((color & 0x1F) << 8) >> 5;
        green = (((color >> 5) & 0x1F) << 8) >> 5;
        blue = (((color >> 10) & 0x1F) << 8) >> 5;

        red = static_cast<int>((red / COLOR_REDUCTION_FACTOR
                                + green / COLOR_INFLUENCE_FACTOR
                                + blue / COLOR_INFLUENCE_FACTOR)
                               + COLOR_INCREMENT_FACTOR);
        green = static_cast<int>(((green / COLOR_REDUCTION_FACTOR)
                                  + (red / COLOR_INFLUENCE_FACTOR)
                                  + (blue / COLOR_INFLUENCE_FACTOR))
                                 + COLOR_INCREMENT_FACTOR);
        blue = static_cast<int>(((blue / COLOR_REDUCTION_FACTOR)
                                 + (red / COLOR_INFLUENCE_FACTOR)
                                 + (green / COLOR_INFLUENCE_FACTOR))
                                + COLOR_INCREMENT_FACTOR);
        gbcColors[color] = static_cast<unsigned int>((red << 16) | (green << 8) | blue);
    }

}

cDisplay::~cDisplay()
{
}

u8 cDisplay::readFromDisplay(u16 a_address)
{
    // TODO: Implement
    if (a_address >= 0xFE00 && a_address < 0xFEA0)
    {
        return mOAM[a_address - 0xFE00];
    }
    return 0;
}

void cDisplay::writeToDisplay(u16 a_address, u8 a_value)
{
    if (a_address >= 0xFE00 && a_address < 0xFEA0)
    {
        mOAM[a_address - 0xFE00] = a_value;
    }
    switch (a_address)
    {
        case 0xFF47://BGP
            BGPTable[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            BGPTable[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            BGPTable[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            BGPTable[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            break;
        case 0xFF48://OBP0
            OBP0Table[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            OBP0Table[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            OBP0Table[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            OBP0Table[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            break;
        case 0xFF49://OBP1
            OBP1Table[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            OBP1Table[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            OBP1Table[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            OBP1Table[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            break;
        case 0xFF68: // BGPI
            mBGPI = a_value;
            break;
        case 0xFF69: // BGPD
            BGColors[mBGPI & 0x3F] = a_value;
            if (mBGPI & 0x80)
                mBGPI = (mBGPI + 1) & 0xFF;
            break;
        case OBPI:
            mOBPI = a_value;
            break;
        case OBPD:
            // 8 color palettes x 4 colors each palette x 2 bytes each color = 64 bytes(0x3F bytes)
            OBJColors[mOBPI & 0x3F] = a_value;
            if (mOBPI & 0x80)
                mOBPI = (mOBPI + 1) & 0xFF;
            break;
    }
}

void cDisplay::hBlankDraw(void)
{//Draws a single line
    unsigned char val;

    val = mem->mem[0xFF40][0];
    lcdc.lcdcActive = (val >> 7) & 1;
    lcdc.wndMap = (((val >> 6) & 1) == 1) ? 0x9C00 : 0x9800;
    lcdc.wndActive = (val >> 5) & 1;
    lcdc.bgWndData = (((val >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    lcdc.bgMap = (((val >> 3) & 1) == 1) ? 0x9C00 : 0x9800;
    lcdc.spActive = (val >> 1) & 1;
    lcdc.bgWndActive = val & 1;
    ly = mem->mem[0xFF44][0];


    if (lcdc.lcdcActive)//If the lcd is on
    {
        if (mIsColor)
        {
            mLCDCPriority = lcdc.bgWndActive;
            drawBackGround();
            if (lcdc.wndActive)
                drawWindow();
            if (lcdc.spActive)
                drawSprites();
        }
        else
        {
            mLCDCPriority = false;
            if (lcdc.bgWndActive)
            {
                drawBackGround();
                if (lcdc.wndActive)
                    drawWindow();
            }
            if (lcdc.spActive)
                drawSprites();
        }
    }
}

void cDisplay::drawBackGround()
{
    s32 x, i, j;
    u8 y;
    u8 tId;
    s32 dir;
    u8 tData1, tData2;
    u8 bank = 0, pal = 0, hFlip = 0, vFlip = 0;
    s32 tileCounter = mem->mem[0xFF43][0] >> 3;

    x = -(mem->mem[0xFF43][0] & 7);
    y = (ly + mem->mem[0xFF42][0]) & 0xFF;

    //Go through all the line tile by tile
    for (i = x; i < 160; i += 8)
    {
        if (tileCounter >= 32)
            tileCounter -= 32;
        //We get the tile Id
        dir = lcdc.bgMap + tileCounter + ((y >> 3) << 5);
        if (mIsColor)
        {
            hFlip = (mem->mem[dir][1] >> 5) & 1;
            vFlip = (mem->mem[dir][1] >> 6) & 1;

            /*dirs[tileCounter][y>>3]=*/mBackgorundPriority = (mem->mem[dir][1] >> 7) & 1;

            bank = (mem->mem[dir][1] >> 3) & 1;
            pal = (mem->mem[dir][1] & 7) << 3;

            BGPTable[0][0] = gbcColors[((BGColors[pal + 1]) << 8) | (BGColors[pal])];
            BGPTable[0][1] = gbcColors[((BGColors[pal + 3]) << 8) | (BGColors[pal + 2])];
            BGPTable[1][0] = gbcColors[((BGColors[pal + 5]) << 8) | (BGColors[pal + 4])];
            BGPTable[1][1] = gbcColors[((BGColors[pal + 7]) << 8) | (BGColors[pal + 6])];
        }
        if (lcdc.bgWndData == 0x8000)
            tId = mem->mem[dir][0];
        else
            tId = (mem->mem[dir][0] ^ 0x80);
        if (vFlip == 1)
        {
            tData1 = mem->mem[lcdc.bgWndData + (tId << 4) + ((7 - (y & 7)) << 1) + 1][bank];
            tData2 = mem->mem[lcdc.bgWndData + (tId << 4) + ((7 - (y & 7)) << 1)][bank];
        }
        else
        {
            tData1 = mem->mem[lcdc.bgWndData + (tId << 4) + ((y & 7) << 1) + 1][bank];
            tData2 = mem->mem[lcdc.bgWndData + (tId << 4) + ((y & 7) << 1)][bank];
        }
        if (hFlip == 1)
        {
            for (j = 0; j < 8; j++)
            {
                if (i + j < 160 && i + j >= 0 && ly < 144 && ly >= 0)
                    videoBuffer[i + j][ly] = BGPTable[tData1 & 1][tData2 & 1];

                tData1 >>= 1;
                tData2 >>= 1;
            }
        }
        else
        {
            for (j = 7; j >= 0; j--)
            {
                if (i + j < 160 && i + j >= 0 && ly < 144 && ly >= 0)
                    videoBuffer[i + j][ly] = BGPTable[tData1 & 1][tData2 & 1];

                tData1 >>= 1;
                tData2 >>= 1;
            }
        }
        ++tileCounter;
    }
}

void cDisplay::drawWindow()
{
    int tileCounter, tId, dir;
    unsigned char tData1, tData2;
    int bank = 0, pal = 0;
    int wx, wy, x, y;
    int i, j;

    wx = mem->mem[0xFF4B][0] - 7;
    wy = mem->mem[0xFF4A][0];

    x = (wx & 7);
    y = (ly - wy) & 0xFF;
    tileCounter = -(wx >> 3);
    if (ly < 144 && ly >= wy && wx <= 166 && wy >= 0 && wy <= 143)
    {
        for (i = x; i < 160; i += 8)
        {
            if (tileCounter >= 32)
                tileCounter -= 32;
            dir = lcdc.wndMap + tileCounter + ((y >> 3) << 5);
            tId = mem->mem[dir][0];
            if (mIsColor)
            {
                bank = (mem->mem[dir][1] >> 3) & 1;
                pal = (mem->mem[dir][1] & 7) << 3;
            }
            if (lcdc.bgWndData == 0x8800)
                tId ^= 0x80;
            tData1 = mem->mem[lcdc.bgWndData + (tId << 4) + ((y & 7) << 1) + 1][bank];
            tData2 = mem->mem[lcdc.bgWndData + (tId << 4) + ((y & 7) << 1)][bank];

            if (mIsColor)
            {
                WPTable[0][0] = gbcColors[((BGColors[pal + 1]) << 8) | (BGColors[pal])];
                WPTable[0][1] = gbcColors[((BGColors[pal + 3]) << 8) | (BGColors[pal + 2])];
                WPTable[1][0] = gbcColors[((BGColors[pal + 5]) << 8) | (BGColors[pal + 4])];
                WPTable[1][1] = gbcColors[((BGColors[pal + 7]) << 8) | (BGColors[pal + 6])];
            }

            for (j = 7; j >= 0; j--)
            {
                if (i + j < 160 && i + j >= 0 && i + j >= wx)
                    videoBuffer[i + j][ly] = WPTable[tData1 & 1][tData2 & 1];
                tData1 >>= 1;
                tData2 >>= 1;
            }
            ++tileCounter;
        }
    }
}

/**
 * OAM is divided into 40 4-byte blocks each of which corresponds to a sprite.
 */
void cDisplay::drawSprites()
{
    int spriteSize{((mem->mem[0xFF40][0] >> 2) & 1) == 1 ? 16 : 8};
    for (int i = 0; i < 40; ++i)
    {
        int spriteFlags{mOAM[i * 4 + 3]};
        mOAMPriority = (spriteFlags & 0x80) != 0;
        int spritePaletteNumber{(spriteFlags >> 4) & 1};
        int spriteBank{0};

        mFinalPriority = 4;

        if (mIsColor)
        {
            spritePaletteNumber = {(spriteFlags & 7) * 8};
            spriteBank = (spriteFlags >> 3) & 1;
            setSpriteColorTable(spritePaletteNumber);
            spritePaletteNumber = 0;
            mFinalPriority = getPriority();
        }

        int spriteY{mOAM[i * 4] - 16};
        if (ly >= spriteY && ly < spriteY + spriteSize)
        {
            bool yFlip{(spriteFlags & 0x40) != 0};
            int yPosition{yFlip ? (spriteSize - 1) - (ly - spriteY) : ly - spriteY};
            int spritePatternNumber{mOAM[(i * 4) + 2]};
            if (spriteSize == 16)
                spritePatternNumber &= 0xFE;
            int patternOffset = spritePatternNumber * 16 + ((yPosition & (spriteSize - 1)) * 2);
            u8 firstByte{mem->mem[0x8000 + patternOffset + 1][spriteBank]};
            u8 secondByte{mem->mem[0x8000 + patternOffset][spriteBank]};
            int spriteX{mOAM[(i * 4) + 1] - 8};
            bool xFlip{(spriteFlags & 0x20) != 0};
            drawSpriteLine(xFlip, spriteX, spritePaletteNumber, firstByte, secondByte);
        }
    }
}

void cDisplay::setSpriteColorTable(int spritePaletteNumber)
{
    OBP0Table[0][0] = gbcColors[((OBJColors[spritePaletteNumber + 1]) << 8) | (OBJColors[spritePaletteNumber])];
    OBP0Table[0][1] = gbcColors[((OBJColors[spritePaletteNumber + 3]) << 8) | (OBJColors[spritePaletteNumber + 2])];
    OBP0Table[1][0] = gbcColors[((OBJColors[spritePaletteNumber + 5]) << 8) | (OBJColors[spritePaletteNumber + 4])];
    OBP0Table[1][1] = gbcColors[((OBJColors[spritePaletteNumber + 7]) << 8) | (OBJColors[spritePaletteNumber + 6])];
}

int cDisplay::getPriority()
{
    if (mLCDCPriority)
    {
        if (!mBackgorundPriority)
        {
            if (mOAMPriority)
                return 5;//OAM behind colors 123
            else
                return 4;//OAM above bg
        }
        else
            return 3;//BG above OAM
    }
    else
        return 0;//BG above everything
}

void cDisplay::drawSpriteLine(bool flipX, int spriteX, int spritePaletteNumber, u8 firstByte, u8 secondByte)
{
    for (int i = 0; i < 8; ++i)
    {
        int p{};
        if (flipX)
            p = i;
        else
            p = 7 - i;

        switch (mFinalPriority)
        {
            case 0:
            case 3:
                break;
            case 4:
                if (isSpritePixelVisible(spriteX, firstByte, secondByte, p))
                    videoBuffer[spriteX + p][ly] = spritePaletteNumber == 1 ? OBP1Table[firstByte & 1][secondByte & 1]
                                                                            : OBP0Table[firstByte & 1][secondByte & 1];
                break;
            case 5:
                if (isSpritePixelVisible(spriteX, firstByte, secondByte, p))
                    videoBuffer[spriteX + p][ly] = spritePaletteNumber == 1 ? OBP1Table[firstByte & 1][secondByte & 1]
                                                                            : OBP0Table[firstByte & 1][secondByte & 1];
                break;
            default:
                //TODO: Raise Exception
                break;
        }
        firstByte >>= 1;
        secondByte >>= 1;
    }
}

bool cDisplay::isSpritePixelVisible(int spriteX, u8 firstByte, u8 secondByte, int p) const
{
    return spriteX + p < 160 && spriteX + p >= 0 && ly >= 0 && ly < 144 && ((firstByte & 1) | (secondByte & 1)) != 0;
}
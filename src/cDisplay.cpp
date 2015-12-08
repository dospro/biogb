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
        BWColors{{0xFFFFFF, 0xC0C0C0},
                 {0x808080, 0x000000}},
        mem{nullptr},
        mMasterPriority{false},
        mIsColor{a_isColor},
        mVRAMBank{0}
{
    mSpriteColorTable = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000, 0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000};
    for (int color = 0; color < 0x10000; ++color)
    {
        int red{((color & 0x1F) << 8) >> 5};
        int green{(((color >> 5) & 0x1F) << 8) >> 5};
        int blue{(((color >> 10) & 0x1F) << 8) >> 5};

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
    mVRAM.push_back(std::array<u8, 0x2000>{});
    if (a_isColor)
    {
        mVRAM.push_back(std::array<u8, 0x2000>{});
    }
}

cDisplay::~cDisplay()
{
}

u8 cDisplay::readFromDisplay(u16 a_address)
{
    // TODO: Implement
    if (a_address >= 0x8000 && a_address < 0xA000)
    {
        return mVRAM[mVRAMBank][a_address - 0x8000];
    }
    else if (a_address >= 0xFE00 && a_address < 0xFEA0)
    {
        return mOAM[a_address - 0xFE00];
    }
    return 0;
}

void cDisplay::writeToDisplay(u16 a_address, u8 a_value)
{
    if (a_address >= 0x8000 && a_address < 0xA000)
    {
        mVRAM[mVRAMBank][a_address - 0x8000] = a_value;
    }
    else if (a_address >= 0xFE00 && a_address < 0xFEA0)
    {
        mOAM[a_address - 0xFE00] = a_value;
    }
    else
    {
        switch (a_address)
        {
            case 0xFF47://BGP
                BGPTable[1][1] = WPTable[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
                BGPTable[1][0] = WPTable[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
                BGPTable[0][1] = WPTable[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
                BGPTable[0][0] = WPTable[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
                break;
            case 0xFF48://OBP0
                for (int i = 0; i < 4; ++i)
                {
                    mSpriteColorTable[i] = BWColors[(a_value >> ((i * 2) + 1)) & 1][(a_value >> (i * 2)) & 1];
                }
                break;
            case 0xFF49://OBP1
                for (int i = 0; i < 4; ++i)
                {
                    mSpriteColorTable[i + 4] = BWColors[(a_value >> ((i * 2) + 1)) & 1][(a_value >> (i * 2)) & 1];
                }
                break;
            case 0xFF4F: // VRAM Bank
                setVRAMBank(a_value);
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
}

void cDisplay::setVRAMBank(int a_bank)
{
    mVRAMBank = a_bank & 1;
}

void cDisplay::hBlankDraw(void)
{//Draws a single line
    unsigned char val;

    val = mem->mem[0xFF40][0];
    lcdc.lcdcActive = (val >> 7) & 1;
    lcdc.wndMap = (((val >> 6) & 1) == 1) ? TILE_MAP_TABLE_1 : TILE_MAP_TABLE_0;
    lcdc.wndActive = (val >> 5) & 1;
    lcdc.tileDataAddress = (((val >> 4) & 1) == 1) ? TILE_PATTERN_TABLE_0 : TILE_PATTERN_TABLE_1;
    lcdc.BGMapAddress = (((val >> 3) & 1) == 1) ? TILE_MAP_TABLE_1 : TILE_MAP_TABLE_0;
    lcdc.spActive = (val >> 1) & 1;
    lcdc.bgWndActive = val & 1;
    ly = mem->mem[0xFF44][0];

    if (ly > 144)
        return;

    if (lcdc.lcdcActive)//If the lcd is on
    {
        if (mIsColor)
        {
            mMasterPriority = lcdc.bgWndActive;
            drawBackGround();
            if (lcdc.wndActive)
                drawWindow();
            if (lcdc.spActive)
                drawSprites();
        }
        else
        {
            mMasterPriority = true;
            if (!lcdc.bgWndActive)
            {
                drawEmptyBG();
            }
            else
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

void cDisplay::drawEmptyBG()
{
    for (int i = 0; i < 160; ++i)
    {
        videoBuffer[i][ly] = 0xFFFFFF;
    }
}

void cDisplay::drawBackGround()
{
    int xScroll{-(mem->mem[0xFF43][0] & 7)};
    int yScroll{(ly + mem->mem[0xFF42][0]) & 0xFF};
    int currentTileInLine = mem->mem[0xFF43][0] / 8;

    for (int i = xScroll; i < 160; i += 8)
    {
        if (currentTileInLine >= 32)
            currentTileInLine -= 32;

        int tileNumber = lcdc.BGMapAddress + ((yScroll / 8) * 32 + currentTileInLine);

        mBackgorundPriority = false;
        bool hFlip{0};
        bool vFlip{0};
        int bank{0};
        if (mIsColor)
        {
            hFlip = ((mVRAM[1][tileNumber] >> 5) & 1) != 0;
            vFlip = ((mVRAM[1][tileNumber] >> 6) & 1) != 0;

            mBackgorundPriority = (mVRAM[1][tileNumber] >> 7) != 0;

            bank = (mVRAM[1][tileNumber] >> 3) & 1;
            setBGColorTable(tileNumber);
        }


        int tileOffset{mVRAM[0][tileNumber]};
        if (lcdc.tileDataAddress == TILE_PATTERN_TABLE_1) //TODO: Corregir el signo
            tileOffset ^= 0x80;

        int offset{lcdc.tileDataAddress + (tileOffset * 16)};
        offset += (vFlip ? (7 - (yScroll & 7)) * 2 : (yScroll & 7) * 2);
        int firstByte{mVRAM[bank][offset + 1]};
        int secondByte{mVRAM[bank][offset]};
        drawTileLine(firstByte, secondByte, i, hFlip);
        ++currentTileInLine;
    }
}

void cDisplay::drawTileLine(int firstByte, int secondByte, int xPosition, bool hFlip)
{
    for (int j = 0; j < 8; j++)
    {
        int xOffset{hFlip ? xPosition + j : xPosition + 7 - j};

        if (isTileVisible(xOffset))
            videoBuffer[xOffset][ly] = BGPTable[firstByte & 1][secondByte & 1];

        firstByte >>= 1;
        secondByte >>= 1;
    }
}

bool cDisplay::isTileVisible(int a_xPosition) const
{
    return a_xPosition < 160 && a_xPosition >= 0;
}

void cDisplay::setBGColorTable(int tileNumber)
{
    int palette = (mVRAM[1][tileNumber] & 7) * 8;

    BGPTable[0][0] = gbcColors[((BGColors[palette + 1]) << 8) | (BGColors[palette])];
    BGPTable[0][1] = gbcColors[((BGColors[palette + 3]) << 8) | (BGColors[palette + 2])];
    BGPTable[1][0] = gbcColors[((BGColors[palette + 5]) << 8) | (BGColors[palette + 4])];
    BGPTable[1][1] = gbcColors[((BGColors[palette + 7]) << 8) | (BGColors[palette + 6])];
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

            tId = mVRAM[0][dir];
            if (mIsColor)
            {
                bank = (mVRAM[1][dir] >> 3) & 1;
                pal = (mVRAM[1][dir] & 7) << 3;
            }
            if (lcdc.tileDataAddress == 0x8800 - 0x8000)
                tId ^= 0x80;
            tData1 = mVRAM[bank][lcdc.tileDataAddress + (tId << 4) + ((y & 7) << 1) + 1];
            tData2 = mVRAM[bank][lcdc.tileDataAddress + (tId << 4) + ((y & 7) << 1)];

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
    int spriteHeight{((mem->mem[0xFF40][0] >> 2) & 1) == 1 ? 15 : 7};
    for (int i = 0; i < TOTAL_OAM_BLOCKS; ++i)
    {
        // TODO: Quitar ly??
        int currentScanLine = ly;
        int yPosition{mOAM[i * 4] - 16};
        if (currentScanLine >= yPosition && currentScanLine <= yPosition + spriteHeight)
        {
            int spriteFlags{mOAM[i * 4 + 3]};
            mOAMPriority = (spriteFlags & 0x80) != 0;
            int spritePaletteNumber{(spriteFlags >> 4) & 1};
            int spriteBank{0};

            mFinalPriority = getPriority();

            if (mIsColor)
            {
                spritePaletteNumber = {(spriteFlags & 7) * 8};
                spriteBank = (spriteFlags >> 3) & 1;
                setSpriteColorTable(spritePaletteNumber);
                spritePaletteNumber = 0;

            }

            bool yFlip{(spriteFlags & 0x40) != 0};
            int yDistance = currentScanLine - yPosition;
            yDistance = yFlip ? spriteHeight - yDistance : yDistance;
            int tileNumber{mOAM[(i * 4) + 2]};
            if (spriteHeight == 15)
                tileNumber &= 0xFE;
            /*
            Each Tile occupies 16 bytes, where each 2 bytes represent a line:
            Byte 0-1  First Line (Upper 8 pixels)
            Byte 2-3  Next Line
            etc.
             */

            int offset{tileNumber * 16 + yDistance * 2};
            u8 firstByte{mVRAM[spriteBank][offset + 1]};
            u8 secondByte{mVRAM[spriteBank][offset]};
            int spriteX{mOAM[(i * 4) + 1] - 8};
            bool xFlip{(spriteFlags & 0x20) != 0};
            drawSpriteLine(xFlip, spriteX, spritePaletteNumber, firstByte, secondByte);
        }
    }
}

void cDisplay::setSpriteColorTable(int a_paletteNumber)
{
    for (int i = 0; i < 4; ++i)
    {
        int colorIndex = ((OBJColors[a_paletteNumber + (i * 2) + 1]) << 8) | (OBJColors[a_paletteNumber + i * 2]);
        mSpriteColorTable[i] = gbcColors[colorIndex];
    }
}

int cDisplay::getPriority()
{
    if (!mMasterPriority)
        return SPRITE_ABOVE_BG; // Sprites van hasta arriba.

    if (mBackgorundPriority)
        return FULL_BG_ABOVE_SPRITE; // BG hasta arriba

    if (!mOAMPriority)
        return SPRITE_ABOVE_BG; // Sprite arriba de BG
    else
        return BG_ABOVE_SPRITE;// BG(123) arriba de sprite
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

        int colorIndex = (((firstByte & 1) << 1) | (secondByte & 1));
        if (mFinalPriority == SPRITE_ABOVE_BG)
        {
            if (isSpritePixelVisible(spriteX, colorIndex, p))
                videoBuffer[spriteX + p][ly] = mSpriteColorTable[4 * spritePaletteNumber + colorIndex];
        }
        else if (mFinalPriority == BG_ABOVE_SPRITE)
        {
            if (isSpritePixelVisible(spriteX, colorIndex, p) &&
                (videoBuffer[spriteX + p][ly] == BGPTable[0][0] || videoBuffer[spriteX + p][ly] == WPTable[0][0]))
                videoBuffer[spriteX + p][ly] = mSpriteColorTable[4 * spritePaletteNumber + colorIndex];
        }
        else
        { ;
        }

        firstByte >>= 1;
        secondByte >>= 1;
    }
}

bool cDisplay::isSpritePixelVisible(int a_xPosition, int a_colorIndex, int a_offset) const
{
    return ((a_xPosition + a_offset) < 160
            && ((a_xPosition + a_offset) >= 0)
            && ly >= 0
            && ly < 144
            && a_colorIndex != 0);
}
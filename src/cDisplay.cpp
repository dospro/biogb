#include"cDisplay.h"

#define COLOR_REDUCTION_FACTOR 1.5
#define COLOR_INFLUENCE_FACTOR 8
#define COLOR_INCREMENT_FACTOR 20

#define BG_WHITE 0xE0F8D0
#define BG_LIGHT_GRAY 0x7EC070
#define GB_DARK_GRAY 0x346856
#define GB_BLACK 0x081820


cDisplay::cDisplay(bool a_isColor) :
        BGPTable{{BG_WHITE,     BG_LIGHT_GRAY},
                 {GB_DARK_GRAY, GB_BLACK}},
        WPTable{{BG_WHITE,     BG_LIGHT_GRAY},
                {GB_DARK_GRAY, GB_BLACK}},
        BWColors{{BG_WHITE,     BG_LIGHT_GRAY},
                 {GB_DARK_GRAY, GB_BLACK}},
        mBGPaletteMemory{},
        mOBJPaletteMemory{},
        mMasterPriority{false},
        mIsColor{a_isColor},
        mVRAMBank{0},
        mTilePrioritiesTable{},
        lcdc{},
        cyclesCounter{0},
        LYCyclesCounter{0},
        nextMode{3},
        LYRegister{0}{
    mSpriteColorTable = {
            BG_WHITE, BG_LIGHT_GRAY, GB_DARK_GRAY, GB_BLACK,
            BG_WHITE, BG_LIGHT_GRAY, GB_DARK_GRAY, GB_BLACK};
    for (int color = 0; color < 0x10000; ++color) {
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
    if (a_isColor) {
        mVRAM.push_back(std::array<u8, 0x2000>{});
    }
    stat.mode = 2;
}

cDisplay::~cDisplay() {}

u8 cDisplay::readFromDisplay(u16 a_address) {
    // TODO: Implement
    if (a_address >= 0x8000 && a_address < 0xA000) {
        return mVRAM[mVRAMBank][a_address - 0x8000];
    } else if (a_address >= 0xFE00 && a_address < 0xFEA0) {
        return mOAM[a_address - 0xFE00];
    } else {
        switch (a_address) {
            case 0xFF40: return lcdc.value;
            case 0xFF41:
                return (stat.LYCInterrupt << 6) |
                       (stat.mode2Interrupt << 5) |
                       (stat.mode1Interrupt << 4) |
                       (stat.mode0Interrupt << 3) |
                       (stat.coincidenceFlag << 2) |
                       stat.mode;
            case 0xFF42: return SCYRegister;
            case 0xFF43: return SCXRegister;
            case 0xFF44: return LYRegister;
            case 0xFF45: return LYCRegister;
            case 0xFF47: return 0;
            case 0xFF48: return 0;
            case 0xFF49: return 0;
            case 0xFF4A: return WYRegister;
            case 0xFF4B: return WXRegister;
            case 0xFF68: return mBGPI;
            case 0xFF69: return mBGPaletteMemory[mBGPI & 0x3F];
            case 0xFF6A: return mOBPI;
            case 0xFF6B: return mOBJPaletteMemory[mOBPI & 0x3F];
            default:break;
        }
    }
    return 0;
}

void cDisplay::writeToDisplay(u16 a_address, u8 a_value) {
    if (a_address >= 0x8000 && a_address < 0xA000) {
        mVRAM[mVRAMBank][a_address - 0x8000] = a_value;
    } else if (a_address >= 0xFE00 && a_address < 0xFEA0) {
        mOAM[a_address - 0xFE00] = a_value;
    } else {
        switch (a_address) {
            case 0xFF40:
                lcdc.value = a_value;
                lcdc.lcdcActive = ((a_value >> 7) & 1) != 0;
                lcdc.windowTileMap = (((a_value >> 6) & 1) == 1) ? TILE_MAP_TABLE_1 : TILE_MAP_TABLE_0;
                lcdc.windowEnabled = ((a_value >> 5) & 1) != 0;
                lcdc.tileDataAddress = (((a_value >> 4) & 1) == 1) ? TILE_PATTERN_TABLE_0 : TILE_PATTERN_TABLE_1;
                lcdc.BGMapAddress = (((a_value >> 3) & 1) == 1) ? TILE_MAP_TABLE_1 : TILE_MAP_TABLE_0;
                lcdc.spriteSize = (((a_value >> 2) & 1) == 1) ? 15 : 7;
                lcdc.spriteEnabled = ((a_value >> 1) & 1) != 0;
                lcdc.bgWndActive = (a_value & 1) != 0;
                break;
            case 0xFF41:
                stat.LYCInterrupt = (a_value & 0x40) != 0;
                stat.mode2Interrupt = (a_value & 0x20) != 0;
                stat.mode1Interrupt = (a_value & 0x10) != 0;
                stat.mode0Interrupt = (a_value & 0x8) != 0;
                break;
            case 0xFF42:
                SCYRegister = a_value;
                break;
            case 0xFF43:
                SCXRegister = a_value;
                break;
            case 0xFF44:
                LYRegister = 0;
                break;
            case 0xFF45:
                LYCRegister = a_value;
                break;
            case 0xFF47://BGP
                BGPTable[1][1] = WPTable[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
                BGPTable[1][0] = WPTable[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
                BGPTable[0][1] = WPTable[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
                BGPTable[0][0] = WPTable[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
                break;
            case 0xFF48://OBP0
                for (int i = 0; i < 4; ++i) {
                    mSpriteColorTable[i] = BWColors[(a_value >> ((i * 2) + 1)) & 1][(a_value >> (i * 2)) & 1];
                }
                break;
            case 0xFF49://OBP1
                for (int i = 0; i < 4; ++i) {
                    mSpriteColorTable[i + 4] = BWColors[(a_value >> ((i * 2) + 1)) & 1][(a_value >> (i * 2)) & 1];
                }
                break;
            case 0xFF4A:
                WYRegister = a_value;
                break;
            case 0xFF4B:
                WXRegister = a_value;
                break;
            case 0xFF4F: // VRAM Bank
                setVRAMBank(a_value);
                break;
            case 0xFF68: // BGPI
                mBGPI = a_value;
                break;
            case 0xFF69: // BGPD
                mBGPaletteMemory[mBGPI & 0x3F] = a_value;
                if (mBGPI & 0x80)
                    mBGPI = (mBGPI + 1) & 0xFF;
                break;
            case OBPI:
                mOBPI = a_value;
                break;
            case OBPD:
                // 8 color palettes x 4 colors each palette x 2 bytes each color = 64 bytes(0x3F bytes)
                mOBJPaletteMemory[mOBPI & 0x3F] = a_value;
                if (mOBPI & 0x80)
                    mOBPI = (mOBPI + 1) & 0xFF;
                break;
            default:
                printf("Writing from IOMap %X: %X\n", a_address, a_value);
                //TODO: Raise exception/
                //std::cout << "Falta: " << a_address << "\n";
                break;
        }
    }
}

void cDisplay::setVRAMBank(int a_bank) {
    mVRAMBank = a_bank & 1;
}

bool cDisplay::hasLineFinished() {
    return isLineFinished;
}


void cDisplay::update(int a_cycles) {
    //NOTE: When CGB is at double speed LCD, Sound and HDMA work as normal.
    //This means those take double clock cycles to finish(because those are
    // slower than the other parts).

    /* Example
     *
     * Let's say that one frame takes 100 cycles. That means the cpu will execute 100 cycles
     * before a render is triggered.
     * In double speed mode, the cpu is able to execute 200 cycles before a render is triggered.
     * In other words, all the cycles for LCDC are doubled.
    */

    // Let's first split the higher level cases:
    // 1. When line changes
    // 2. When all 144 lines are drawn
    // 3. Do nothing for 10 lines???


    const auto cyclesPerMode0 = 206;
    const auto cyclesPerMode2 = 82;
    const auto cyclesPerMode3 = 168;
    const auto cyclesPerLines = cyclesPerMode0 + cyclesPerMode2 + cyclesPerMode3;  // 456

    isLineFinished = false;
    if (!lcdc.lcdcActive) {
        return;
    }

    LYCyclesCounter += a_cycles;

    if (stat.mode == 2) {
        if (LYCyclesCounter >= cyclesPerMode2) {
            // Mode 2 finished, next mode is 3
            stat.mode = 3;
        }
    } else if (stat.mode == 3) {
        if (LYCyclesCounter >= cyclesPerMode2 + cyclesPerMode3) {
            stat.mode = 0;
            if (stat.mode0Interrupt) {
                mLCDInterruptRequest = true;
            }
            hBlankDraw();
        }
    } else if (stat.mode == 0 || stat.mode == 1) {
        if (LYCyclesCounter >= cyclesPerLines) {
            LYCyclesCounter -= cyclesPerLines;
            LYRegister++;
            isLineFinished = true;

            // TODO: Maybe modify cycles for lines 153 and 0???

            if (LYRegister < 144) {
                stat.mode = 2;
                if (stat.mode2Interrupt) {
                    mLCDInterruptRequest = true;
                }
            } else if (LYRegister == 144) {
                // From line 144 to 153 we are on vblank
                stat.mode = 1;
                if ( stat.mode1Interrupt ) {
                    mLCDInterruptRequest = true;
                }
                mVBlankInterruptRequest = true;
            } else if (LYRegister > 153) {
                // After line 153 we start all over again
                LYRegister = 0;
                stat.mode = 2;
            }
//            printf("Current line %d\n", LYRegister);

            stat.coincidenceFlag = false;
            if (LYRegister == LYCRegister) {
                stat.coincidenceFlag = true;
                if (stat.LYCInterrupt) {
                    mLCDInterruptRequest = true;
                }
            }
        }
    }
}

const cDisplay::VideoBuffer &cDisplay::getFrameBuffer() {
    return this->videoBuffer;
}

void cDisplay::hBlankDraw() {
    if (lcdc.lcdcActive) {
        if (mIsColor) {
            mMasterPriority = lcdc.bgWndActive;
            drawBackGround();
            if (lcdc.windowEnabled)
                drawWindow();
            if (lcdc.spriteEnabled)
                drawSprites();
        } else {
            mMasterPriority = true;
            if (!lcdc.bgWndActive) {
                drawEmptyBG();
            } else {
                drawBackGround();
                if (lcdc.windowEnabled)
                    drawWindow();
            }
            if (lcdc.spriteEnabled)
                drawSprites();
        }
    }
}

void cDisplay::drawEmptyBG() {
    for (int i = 0; i < 160; ++i) {
        videoBuffer[LYRegister][i] = 0xFFFFFF;
    }
}

void cDisplay::drawBackGround() {
    int xScroll{-(SCXRegister & 7)};
    int yScroll{(LYRegister + SCYRegister) & 0xFF};
    int currentTileInLine = SCXRegister / 8;

    for (int i = xScroll; i < 160; i += 8) {
        if (currentTileInLine >= 32)
            currentTileInLine -= 32;

        int tileNumber = lcdc.BGMapAddress + ((yScroll / 8) * 32 + currentTileInLine);

        //mTilePrioritiesTable[counter]
        bool hFlip{0};
        bool vFlip{0};
        int bank{0};
        if (mIsColor) {
            hFlip = ((mVRAM[1][tileNumber] >> 5) & 1) != 0;
            vFlip = ((mVRAM[1][tileNumber] >> 6) & 1) != 0;

            mTilePrioritiesTable[(i - xScroll) / 8] = (mVRAM[1][tileNumber] >> 7) != 0;

            bank = (mVRAM[1][tileNumber] >> 3) & 1;
            setBGColorTable(tileNumber);
        }


        int tileOffset{mVRAM[0][tileNumber]};
        if (lcdc.tileDataAddress == TILE_PATTERN_TABLE_1) {
            tileOffset ^= 0x80;
        }

        int offset{lcdc.tileDataAddress + (tileOffset * 16)};
        offset += (vFlip ? (7 - (yScroll & 7)) * 2 : (yScroll & 7) * 2);
        int firstByte{mVRAM[bank][offset + 1]};
        int secondByte{mVRAM[bank][offset]};
        drawTileLine(firstByte, secondByte, i, hFlip);
        ++currentTileInLine;
    }
}

void cDisplay::drawTileLine(int firstByte, int secondByte, int xPosition, bool hFlip) {
    for (int j = 0; j < 8; j++) {
        int xOffset{hFlip ? xPosition + j : xPosition + 7 - j};
        if (isTileVisible(xOffset)) {
            videoBuffer[LYRegister][xOffset] = BGPTable[firstByte & 1u][secondByte & 1u];
        }
        firstByte >>= 1;
        secondByte >>= 1;
    }
}

bool cDisplay::isTileVisible(int a_xPosition) const {
    return a_xPosition < 160 && a_xPosition >= 0 && LYRegister >= 0 && LYRegister < 144;
}

void cDisplay::setBGColorTable(int tileNumber) {
    int palette = (mVRAM[1][tileNumber] & 7) * 8;

    BGPTable[0][0] = gbcColors[((mBGPaletteMemory[palette + 1]) << 8) | (mBGPaletteMemory[palette])];
    BGPTable[0][1] = gbcColors[((mBGPaletteMemory[palette + 3]) << 8) | (mBGPaletteMemory[palette + 2])];
    BGPTable[1][0] = gbcColors[((mBGPaletteMemory[palette + 5]) << 8) | (mBGPaletteMemory[palette + 4])];
    BGPTable[1][1] = gbcColors[((mBGPaletteMemory[palette + 7]) << 8) | (mBGPaletteMemory[palette + 6])];
}

void cDisplay::drawWindow() {
    int wx{WXRegister - 7};
    int wy{WYRegister};
    int y{(LYRegister - wy) & 0xFF};

    if (LYRegister >= wy && wx < 160 && wy >= 0 && wy < 144) {
        int tileInLine = {-(wx / 8)};
        for (int i = wx & 7; i < 160; i += 8) {
            if (tileInLine >= 32)
                tileInLine -= 32;
            int tileNumber{lcdc.windowTileMap + tileInLine + ((y / 8) * 32)};
            int tileOffset{mVRAM[0][tileNumber]};
            int bank{0};
            bool horizontalFlip{};
            bool verticalFlip{};
            if (mIsColor) {
                int tileAttribute = mVRAM[1][tileNumber];
                horizontalFlip = {((tileAttribute >> 5) & 1) != 0};
                verticalFlip = {((tileAttribute >> 6) & 1) != 0};
                bank = (tileAttribute >> 3) & 1;

                mTilePrioritiesTable[i / 8] = (tileAttribute >> 7) != 0;

                int palette = (tileAttribute & 7) * 8;
                WPTable[0][0] = gbcColors[((mBGPaletteMemory[palette + 1]) << 8) | (mBGPaletteMemory[palette])];
                WPTable[0][1] = gbcColors[((mBGPaletteMemory[palette + 3]) << 8) | (mBGPaletteMemory[palette + 2])];
                WPTable[1][0] = gbcColors[((mBGPaletteMemory[palette + 5]) << 8) | (mBGPaletteMemory[palette + 4])];
                WPTable[1][1] = gbcColors[((mBGPaletteMemory[palette + 7]) << 8) | (mBGPaletteMemory[palette + 6])];
            }
            if (lcdc.tileDataAddress == TILE_PATTERN_TABLE_1)
                tileOffset ^= 0x80;
            int finalOffset{lcdc.tileDataAddress + (tileOffset * 16)};
            finalOffset += verticalFlip ? (7 - (y & 7)) * 2 : (y & 7) * 2;
            int firstByte = mVRAM[bank][finalOffset + 1];
            int secondByte = mVRAM[bank][finalOffset];

            for (int j = 7; j >= 0; j--) {
                int p{horizontalFlip ? i + (7 - j) : i + j};
                if (p < 160 && p >= 0 && p >= wx)
                    videoBuffer[LYRegister][p] = WPTable[firstByte & 1][secondByte & 1];
                firstByte >>= 1;
                secondByte >>= 1;
            }
            ++tileInLine;
        }
    }
}

/**
 * OAM is divided into 40 4-byte blocks each of which corresponds to a sprite.
 */
void cDisplay::drawSprites() {
    for (int i = TOTAL_OAM_BLOCKS - 1; i >= 0; --i) {
        int yPosition{mOAM[i * 4] - 16};
        if (LYRegister >= yPosition && LYRegister <= yPosition + lcdc.spriteSize) {
            int spriteFlags{mOAM[i * 4 + 3]};
            mOAMPriority = (spriteFlags & 0x80) != 0;
            int spritePaletteNumber{(spriteFlags >> 4) & 1};
            int spriteBank{0};

            mFinalPriority = getPriority();

            if (mIsColor) {
                spritePaletteNumber = {(spriteFlags & 7) * 8};
                spriteBank = (spriteFlags >> 3) & 1;
                setSpriteColorTable(spritePaletteNumber);
                spritePaletteNumber = 0;

            }

            bool yFlip{(spriteFlags & 0x40) != 0};
            int yDistance = LYRegister - yPosition;
            yDistance = yFlip ? lcdc.spriteSize - yDistance : yDistance;
            int tileNumber{mOAM[(i * 4) + 2]};
            if (lcdc.spriteSize == 15) {
                tileNumber &= 0xFE;
            }
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

void cDisplay::setSpriteColorTable(int a_paletteNumber) {
    for (int i = 0; i < 4; ++i) {
        int colorIndex = ((mOBJPaletteMemory[a_paletteNumber + (i * 2) + 1]) << 8) |
                         (mOBJPaletteMemory[a_paletteNumber + i * 2]);
        mSpriteColorTable[i] = gbcColors[colorIndex];
    }
}

int cDisplay::getPriority() {
    if (!mMasterPriority)
        return SPRITE_ABOVE_BG; // Sprites van hasta arriba.

    if (!mOAMPriority)
        return OAM_SPRITE_ABOVE_BG; // Sprite arriba de BG
    else
        return BG_ABOVE_SPRITE;// BG(123) arriba de sprite
}

void cDisplay::drawSpriteLine(bool flipX, int spriteX, int spritePaletteNumber, u8 firstByte, u8 secondByte) {
    for (int i = 0; i < 8; ++i) {
        int offset{flipX ? i : 7 - i};

        auto colorIndex = (((firstByte & 1u) << 1u) | (secondByte & 1u));
        if (mFinalPriority == SPRITE_ABOVE_BG) {
            if (isSpritePixelVisible(spriteX, colorIndex, offset))
                videoBuffer[LYRegister][spriteX + offset] = mSpriteColorTable[4 * spritePaletteNumber + colorIndex];
        } else if (mFinalPriority == OAM_SPRITE_ABOVE_BG) {
            if (!mTilePrioritiesTable[(spriteX + offset) / 8] && isSpritePixelVisible(spriteX, colorIndex, offset))
                videoBuffer[LYRegister][spriteX + offset] = mSpriteColorTable[4 * spritePaletteNumber + colorIndex];
        } else if (mFinalPriority == BG_ABOVE_SPRITE) {
            if (isSpritePixelVisible(spriteX, colorIndex, offset) &&
                (videoBuffer[LYRegister][spriteX + offset] == BGPTable[0][0] ||
                 videoBuffer[LYRegister][spriteX + offset] == WPTable[0][0]))
                videoBuffer[LYRegister][spriteX + offset] = mSpriteColorTable[4 * spritePaletteNumber + colorIndex];
        } else {
        }

        firstByte >>= 1u;
        secondByte >>= 1u;
    }
}

bool cDisplay::isSpritePixelVisible(int a_xPosition, int a_colorIndex, int a_offset) const {
    return ((a_xPosition + a_offset) < 160
            && ((a_xPosition + a_offset) >= 0)
            && a_colorIndex != 0);
}
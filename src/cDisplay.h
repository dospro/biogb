#ifndef BIOGB_GFX
#define BIOGB_GFX

#include <array>
#include <vector>
#include"macros.h"

#define BGPI 0xFF68
#define BGPD 0xFF69
#define OBPI 0xFF6A
#define OBPD 0xFF6B


class MemoryMap;

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

    void getMemoryPointer(MemoryMap *a_memory)
    { memory = a_memory; };
    void hBlankDraw(void);

protected:
    const int mScreenWidth = 160;
    const int mScreenHeight = 144;
    int videoBuffer[144][160];

private:
    const int TOTAL_OAM_BLOCKS = 40;
    const int SPRITE_ABOVE_BG = 0;
    const int BG_ABOVE_SPRITE = 1;
    const int OAM_SPRITE_ABOVE_BG = 2;
    const int TILE_PATTERN_TABLE_0 = 0; //0x8000 - 0x8000;
    const int TILE_PATTERN_TABLE_1 = 0x0800; //0x8800 - 0x8000;
    const int TILE_MAP_TABLE_0 = 0x1800; // 0x9800 - 0x8000
    const int TILE_MAP_TABLE_1 = 0x1C00; // 0x9C00 - 0x8000
    MemoryMap *memory;
    std::array<u8, 0x9F> mOAM;
    std::vector<std::array<u8, 0x2000>> mVRAM;
    std::array<u32, 8> mSpriteColorTable;
    std::array<bool, 32> mTilePrioritiesTable;
    int mVRAMBank;
    u32 gbcColors[0x10000];
    u32 BWColors[2][2];
    u32 BGPTable[2][2];
    u32 WPTable[2][2];
    u32 mBGPaletteMemory[64];
    u32 mOBJPaletteMemory[64];
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

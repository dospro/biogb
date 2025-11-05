#ifndef BIOGB_GFX
#define BIOGB_GFX

#include <array>
#include <vector>
#include <mdspan>

#include "macros.h"

class MemoryMap;

struct LCDC {
    u8 value;
    bool lcdcActive;
    int windowTileMap;
    bool windowEnabled;
    int tileDataAddress;
    int BGMapAddress;
    int spriteSize;
    bool spriteEnabled;
    bool bgWndActive;
};

struct STAT {
    bool LYCInterrupt;
    bool mode2Interrupt;
    bool mode1Interrupt;
    bool mode0Interrupt;
    bool coincidenceFlag;
    unsigned int mode;
};


class cDisplay {
   public:
    explicit cDisplay(bool a_isColor);
    ~cDisplay() = default;
    [[nodiscard]] u8 readFromDisplay(u16 a_address) const;
    void writeToDisplay(u16 a_address, u8 a_value);
    void setVRAMBank(int a_bank) noexcept;
    [[nodiscard]] bool hasLineFinished() const noexcept;

    void hBlankDraw();
    void update(int a_cycles);
    [[nodiscard]] std::span<const u32> get_video_buffer() const;

    bool mVBlankInterruptRequest{};
    bool mLCDInterruptRequest{};

   protected:
    static constexpr int mScreenWidth = 160;
    static constexpr int mScreenHeight = 144;
    std::array<u32, 160 * 144> video_buffer_data{};
    std::mdspan<u32, std::extents<size_t, 144, 160> > videoBuffer{video_buffer_data.data()};

   private:
    static constexpr u32 BG_WHITE = 0xE0F8D0;
    static constexpr u32 BG_LIGHT_GRAY = 0x7EC070;
    static constexpr u32 GB_DARK_GRAY = 0x346856;
    static constexpr u32 GB_BLACK = 0x081820;
    static constexpr int TOTAL_OAM_BLOCKS = 40;
    static constexpr int SPRITE_ABOVE_BG = 0;
    static constexpr int BG_ABOVE_SPRITE = 1;
    static constexpr int OAM_SPRITE_ABOVE_BG = 2;
    static constexpr int TILE_PATTERN_TABLE_0 = 0;       // 0x8000 - 0x8000;
    static constexpr int TILE_PATTERN_TABLE_1 = 0x0800;  // 0x8800 - 0x8000;
    static constexpr int TILE_MAP_TABLE_0 = 0x1800;      // 0x9800 - 0x8000
    static constexpr int TILE_MAP_TABLE_1 = 0x1C00;      // 0x9C00 - 0x8000
    std::array<u8, 0xA0> mOAM{};
    std::vector<std::array<u8, 0x2000>> mVRAM;
    std::array<u32, 8> mSpriteColorTable{
        BG_WHITE, BG_LIGHT_GRAY, GB_DARK_GRAY, GB_BLACK,
        BG_WHITE, BG_LIGHT_GRAY, GB_DARK_GRAY, GB_BLACK
    };
    std::array<bool, 32> mTilePrioritiesTable{};
    int mVRAMBank{};
    std::array<u32, 0x10000> gbcColors{};
    std::array<std::array<u32, 2>,2> BWColors{{
        {BG_WHITE, BG_LIGHT_GRAY},
        {GB_DARK_GRAY, GB_BLACK}
    }};
    std::array<std::array<u32, 2>,2> BGPTable{{
        {BG_WHITE, BG_LIGHT_GRAY},
        {GB_DARK_GRAY, GB_BLACK}
    }};
    std::array<std::array<u32, 2>,2> WPTable{{
        {BG_WHITE, BG_LIGHT_GRAY},
        {GB_DARK_GRAY, GB_BLACK}
    }};
    std::array<u32, 64> mBGPaletteMemory{};
    std::array<u32, 64> mOBJPaletteMemory{};
    LCDC lcdc{};
    STAT stat{};
    u8 LYRegister{};
    u8 LYCRegister{};
    u8 SCXRegister{};
    u8 SCYRegister{};
    u8 WXRegister{};
    u8 WYRegister{};
    u8 BGPRegister{};
    u8 OBP0Register{};
    u8 OBP1Register{};
    int LYCyclesCounter{};
    int cyclesCounter{};
    u32 nextMode{3};
    int mFinalPriority{};
    int BGPIRegister{};
    int OBPIRegister{};
    bool mIsColor;
    bool isLineFinished{};
    bool mMasterPriority{false}, mOAMPriority{};
    void drawBackGround();
    void drawEmptyBG() const;
    void drawWindow();
    void drawSprites();
    int getPriority() const;
    void drawSpriteLine(bool flipX, int spriteX, int spritePaletteNumber, u8 firstByte, u8 secondByte) const;
    bool isSpritePixelVisible(int a_xPosition, int colorIndex, int a_offset) const;
    void setSpriteColorTable(int a_paletteNumber);
    void setBGColorTable(int tileNumber);
    bool isTileVisible(int a_xPosition) const noexcept;
    void drawTileLine(int firstByte, int secondByte, int xPosition, bool hFlip) const;

    void setBGPColors(u8 value);
};

#endif

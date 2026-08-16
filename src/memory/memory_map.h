#ifndef BIOGB_MEMORY_MAP_H
#define BIOGB_MEMORY_MAP_H

#include <array>
#include <expected>
#include <string>
#include <vector>

#include "rom_loader.h"
#include "../display.h"
#include "../input.h"
#include "../timer.h"
#include "../macros.h"
#include "../sound/sound.h"
#include "../sgb.h"

enum eInterrupts { VBLANK = 1, LCDC = 2, TIMER = 4, SERIAL = 8, JOYPAD = 16 };

struct HDMA {
    u16 src;
    u16 dest;
    int length;
    bool mode, active;
};

struct SerialTransfer {
    bool start;
    bool cType;
    bool speed;
    u8 rec, trans;
};

class cTimer;

class MemoryMap {
   public:
    MemoryMap() = default;
    ~MemoryMap() = default;
    std::expected<void, std::string> load_rom(std::string_view file_name);
    [[nodiscard]] ConsoleModel console_model() const { return model; }
    [[nodiscard]] bool is_sgb() const { return model == ConsoleModel::SGB; }
    u8 readByte(u16);
    void writeByte(u16, u8);
    void HBlankHDMA();
    void save_sram();
    void load_sram();
    void updateIO(int a_cycles);
    int changeSpeed();
    u8 getEnabledInterrupts();
    void resetInterruptRequest(int interrupt);
    int readIFRegister();
    void writeIFRegister(u8 value);

    [[nodiscard]] std::span<const u32> get_sgb_buffer(std::span<const u32> gb_frame);
    void execute_sgb_vram_transfer();

    std::unique_ptr<cDisplay> mDisplay{};
    std::unique_ptr<cSound> mSound{};
    std::unique_ptr<cInput> mInput{};
    std::unique_ptr<cTimer> mTimer{};

    SerialTransfer ST{};
    u16 wRamBank{1};
    std::array<u8, 0x100> IOMap{};

   private:
    Cartridge cartridge;
    bool is_battery_backed = false;
    bool has_rtc = false;
    std::vector<std::array<u8, 0x1000>> mWRam{};
    std::array<u8, 0x80> mHRam{};
    std::string mRomFilename{};
    HDMA hdma{};
    bool mIsColor{};
    ConsoleModel model{};
    int mCurrentSpeed{};
    bool mPrepareSpeedChange{};
    u8 IERegister{};
    SGB sgb{};
    void init_wram(bool is_color);
    [[nodiscard]] std::expected<void, std::string> init_sub_systems() noexcept;
    void DMATransfer(u8 address);
    void HDMATransfer(u16 source, u16 dest, u32 length);
    [[nodiscard]] u8 readRom(u16 a_address) const noexcept;
    [[nodiscard]] u8 readRam(u16 address) const;
    void writeIO(u16 a_address, u8 a_value);
    int readIO(int a_address);
};

#endif  // BIOGB_MEMORY_MAP_H

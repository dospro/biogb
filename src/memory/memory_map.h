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

enum eInterrupts { VBLANK = 1, LCDC = 2, TIMER = 4, SERIAL = 8, JOYPAD = 16 };

struct HDMA {
    u16 src;
    u16 dest;
    int length;
    bool mode, active;
};
struct RTC_Regs {
    u8 rtcRegSelect;
    bool areRtcRegsSelected;
    u8 latch;
    u8 sec;
    u8 min;
    u8 hr;
    u8 dl;
    u8 dh;
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
    void rtcCounter();
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

    std::unique_ptr<cDisplay> mDisplay{};
    std::unique_ptr<cSound> mSound{};
    std::unique_ptr<cInput> mInput{};
    std::unique_ptr<cTimer> mTimer{};
    RTC_Regs rtc{}, rtc2{};
    SerialTransfer ST{};
    u16 romBank{1}, ramBank{}, wRamBank{1};
    std::array<u8, 0x100> IOMap{};

   private:
    std::vector<std::array<u8, 0x4000>> mRom{};
    std::vector<std::array<u8, 0x2000>> mRam{};
    std::vector<std::array<u8, 0x1000>> mWRam{};
    std::array<u8, 0x80> mHRam{};
    std::string mRomFilename{};
    HDMA hdma{};
    bool mIsColor{};
    bool mRomMode{};
    u8 MBC5HighAddress{}, MBC5LowAddress{};
    int mCurrentSpeed{};
    bool mPrepareSpeedChange{};
    MBCTypes mbc_type{};
    bool with_timer{};
    u8 IERegister{};
    void init_ram(int ram_banks);
    void init_wram(bool is_color);
    [[nodiscard]] std::expected<void, std::string> init_sub_systems() noexcept;
    void DMATransfer(u8 address);
    void HDMATransfer(u16 source, u16 dest, u32 length);
    u8 readRom(u16 a_address) const noexcept;
    u8 readRam(u16 address) const;
    virtual void send_command(u16 address, u8 value);
    void sendMBC1Command(u16 a_address, u8 a_value);
    void sendMBC2Command(u16 a_address, u8 a_value);
    void sendMBC3Command(u16 a_address, u8 a_value);
    void sendMBC5Command(u16 a_address, u8 a_value);
    void writeRTCRegister(u8 a_value);
    void writeIO(u16 a_address, u8 a_value);
    int readIO(int a_address);
};

#endif  // BIOGB_MEMORY_MAP_H

#ifndef BIOGB_MEMORYMAP_H
#define BIOGB_MEMORYMAP_H

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "macros.h"
#include "cDisplay.h"
#include "sound/cSound.h"
#include "cInput.h"
#include "cInterrupts.h"
#include "cTimer.h"
#include "Memory/RomLoader.h"

struct HDMA
{
    u8 hs, ls;
    u8 hd, ld;
    int length;
    bool mode, active;
};
struct RTC_Regs
{
    u8 rtcRegSelect;
    bool areRtcRegsSelected;
    bool latch;
    u8 sec;
    u8 min;
    u8 hr;
    u8 dl;
    u8 dh;
};
struct SerialTransfer
{
    bool start;
    bool cType;
    bool speed;
    u8 rec, trans;
};

class MemoryMap
{
public:
    MemoryMap();
    ~MemoryMap();
    void rtcCounter(void);
    bool load_rom(const std::string& file_name);
    u8 readByte(u16);
    void writeByte(u16, u8);
    void HBlankHDMA();
    void save_sram();
    void load_sram();
    void updateIO(int a_cycles);
    int changeSpeed();

    std::unique_ptr<cDisplay> mDisplay;
    std::unique_ptr<cSound> mSound;
    std::unique_ptr<cInput> mInput;
    std::unique_ptr<cInterrupts> mInterrupts;
    std::unique_ptr<cTimer> mTimer;
    RTC_Regs rtc, rtc2;
    SerialTransfer ST;
    u16 romBank, ramBank, wRamBank;
    u8 IOMap[0x10000][1]{};

private:
    std::vector<std::array<u8, 0x4000>> mRom;
    std::vector<std::array<u8, 0x2000>> mRam;
    std::vector<std::array<u8, 0x1000>> mWRam;
    std::array<u8, 0x80> mHRam{};
    std::string mRomFilename;
    HDMA hdma;
    bool mRomMode;
    u8 hi, lo;
    int dest, source;
    int mCurrentSpeed;
    bool mPrepareSpeedChange{};
    MBCTypes mbc_type;
    bool with_timer{};
    void init_ram(int ram_banks);
    void init_wram(bool is_color);
    void init_sub_systems();
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


#endif //BIOGB_MEMORYMAP_H

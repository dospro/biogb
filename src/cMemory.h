//
// Created by dospro on 25/11/15.
//

#ifndef BIOGB_CMEMORY_H
#define BIOGB_CMEMORY_H

#include <vector>
#include <array>
#include "macros.h"
#include "cDisplay.h"
#include "sound/cSound.h"
#include "cInput.h"
#include "cInterrupts.h"
#include "cTimer.h"

struct gbHeader
{
    char name[16];
    u8 color;
    u8 mbc;
    u8 romSize;
    u8 ramSize;
};
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

class cMemory
{
public:
    cDisplay *mDisplay;
    cSound *mSound;
    cInput *mInput;
    cInterrupts *mInterrupts;
    cTimer *mTimer;
    gbHeader info;
    RTC_Regs rtc, rtc2;
    SerialTransfer ST;
    u16 romBank, ramBank, wRamBank;
    u8 IOMap[0x10000][1];
    cMemory();
    ~cMemory();
    void rtcCounter(void);
    bool loadRom(const char *file);
    u8 readByte(u16);
    void writeByte(u16, u8);
    void HBlankHDMA();
    void saveSram();
    void loadSram();
    void updateIO(int a_cycles);
    int changeSpeed();

private:
    std::vector<std::array<u8, 0x4000>> mRom;
    std::vector<std::array<u8, 0x2000>> mRam;
    std::vector<std::array<u8, 0x1000>> mWRam;
    std::array<u8, 0x80> mHRam;
    std::string mRomFilename;
    HDMA hdma;
    u8 mm;
    u8 hi, lo;
    int dest, source;
    int mCurrentSpeed;
    bool mPrepareSpeedChange;
    void DMATransfer(u8 address);
    void HDMATransfer(u16 source, u16 dest, u32 length);
    u8 readRom(u16 a_address) const noexcept;
    u8 readRam(u16 address) const;
    bool isMBC1(gbHeader a_header) const;
    bool isMBC2(gbHeader a_header) const;
    bool isMBC3(gbHeader a_header) const;
    bool isMBC5(gbHeader a_header) const;
    void sendMBC1Command(u16 a_address, u8 a_value);
    void sendMBC2Command(u16 a_address, u8 a_value);
    void sendMBC3Command(u16 a_address, u8 a_value);
    void sendMBC5Command(u16 a_address, u8 a_value);
    void writeRTCRegister(u8 a_value);
    void writeIO(u16 a_address, u8 a_value);
    int readIO(int a_address);
};


#endif //BIOGB_CMEMORY_H

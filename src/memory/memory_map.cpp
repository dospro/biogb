#include "memory_map.h"

#include <iostream>

#include "../custom_exceptions.h"
#include "../imp/audio/sdl_audio.h"
#include "../imp/video/sdl_display.h"

MemoryMap::MemoryMap()
    : mCurrentSpeed{0},
      mDisplay{},
      mSound{},
      mInput{},
      mTimer{},
      romBank{1},
      ramBank{},
      wRamBank{1},
      mRomMode{},
      MBC5HighAddress{},
      MBC5LowAddress{},
      hdma{},
      rtc{},
      rtc2{},
      ST{} {}

MemoryMap::~MemoryMap() = default;

bool MemoryMap::load_rom(const std::string &file_name) {
    mRomFilename = std::string(file_name);
    RomLoader loader{file_name};
    mRom = loader.get_rom();
    mIsColor = loader.is_color();
    mbc_type = loader.get_mbc_type();
    with_timer = loader.has_timer();
    init_ram(loader.get_ram_banks());
    init_wram(loader.is_color());
    load_sram();
    init_sub_systems();
    return true;
}

void MemoryMap::init_ram(int ram_banks) {
    for (int i = 0; i < ram_banks; ++i) {
        mRam.emplace_back(std::array<u8, 0x2000>{});
    }
}

void MemoryMap::init_wram(bool is_color) {
    mWRam.push_back(std::array<u8, 0x1000>{});
    mWRam.push_back(std::array<u8, 0x1000>{});
    if (is_color) {
        for (int i = 0; i < 6; ++i) {
            mWRam.push_back(std::array<u8, 0x1000>{});
        }
    }
}

void MemoryMap::init_sub_systems() {
    std::cout << "Display.";
    mDisplay = std::make_unique<cSDLDisplay>(mIsColor);
    std::cout << "OK" << std::endl;

    std::cout << "Sound...";
    mSound = std::make_unique<cSDLSound>(44100, 512);
    if (!mSound) {
        throw SoundSystemError("Failed to initialize sound system");
    }
    std::cout << "OK" << std::endl;
    std::cout << "Turning sound on" << std::endl;
    mSound->turnOn();

    std::cout << "Input...";
    mInput = std::make_unique<cInput>();
    std::cout << "OK" << std::endl;
    mTimer = std::make_unique<cTimer>();
}

u8 MemoryMap::readByte(u16 address) {
    if (address < 0x8000) return readRom(address);
    else if (address < 0xA000)
        return mDisplay->readFromDisplay(address);
    else if (address < 0xC000)
        return readRam(address);
    else if (address < 0xD000)
        return mWRam[0][address - 0xC000];
    else if (address < 0xE000)
        return mWRam[wRamBank][address - 0xD000];
    else if (address < 0xFE00)
        return readByte(address - 0x2000);
    else if (address < 0xFEA0)
        return mDisplay->readFromDisplay(address);
    else if (address < 0xFF00)
        ;
    else if (address < 0xFF01)
        return mInput->readRegister();
    else if (address < 0xFF10)
        return readIO(address);
    else if (address < 0xFF40)
        return mSound->readFromSound(address);
    else if (address < 0xFF80)
        return readIO(address);
    else if (address < 0xFFFE)
        return mHRam[address - 0xFF80];
    else
        return readIO(address);

    return 0xFF;
}

int MemoryMap::readIO(int a_address) {
    switch (a_address) {
        case 0xFF04:
        case 0xFF05:
        case 0xFF06:
        case 0xFF07: return mTimer->readRegister(a_address);
        case 0xFF0F: return readIFRegister();
        case 0xFF4D: return (static_cast<int>(mPrepareSpeedChange)) | (mCurrentSpeed << 7);
        case 0xFF40:  // LCDC
        case 0xFF41:  // STAT
        case 0xFF42:  // SCY
        case 0xFF43:  // SCX
        case 0xFF44:  // LY
        case 0xFF45:  // LYC
        case 0xFF47:  // BGP
        case 0xFF48:  // OBP0
        case 0xFF49:  // OBP1
        case 0xFF4A:  // WY
        case 0xFF4B:  // WX
        case 0xFF4F:  // VRAM Bank
        case 0xFF68:  // BGPI
        case 0xFF69:  // BGPD
        case 0xFF6A:
        case 0xFF6B: return mDisplay->readFromDisplay(a_address);
        case 0xFF51: return hdma.src & 0xFF;
        case 0xFF52: return (hdma.src >> 8) & 0xFF;
        case 0xFF53: return hdma.dest & 0xFF;
        case 0xFF54: return (hdma.dest >> 8) & 0xFF;
        case 0xFF55: return (hdma.mode << 7) | (hdma.length / 0x10 - 1);
        case 0xFFFF: return IERegister;
        default: return IOMap[a_address][0];
    }
}

u8 MemoryMap::readRom(u16 a_address) const noexcept {
    if (a_address < 0x4000) return mRom[0][a_address];
    else
        return mRom[romBank][a_address - 0x4000];
}

u8 MemoryMap::readRam(u16 address) const {
    if (rtc.areRtcRegsSelected) {
        switch (rtc.rtcRegSelect) {
            case 0x8: return rtc.sec;
            case 0x9: return rtc2.min;
            case 0xA: return rtc2.hr;
            case 0xB: return rtc2.dl;
            case 0xC: return rtc2.dh;
            default: return mRam[ramBank][address - 0xA000];
        }
    } else
        return mRam[ramBank][address - 0xA000];
}

void MemoryMap::send_command(u16 address, u8 value) {
    if (mbc_type == MBCTypes::MBC1) {
        sendMBC1Command(address, value);
    } else if (mbc_type == MBCTypes::MBC2) {
        sendMBC2Command(address, value);
    } else if (mbc_type == MBCTypes::MBC3) {
        sendMBC3Command(address, value);
    } else if (mbc_type == MBCTypes::MBC5) {
        sendMBC5Command(address, value);
    }
}

void MemoryMap::writeByte(u16 a_address, u8 a_value) {
    if (a_address < 0x8000) {
        send_command(a_address, a_value);
    } else if (a_address < 0xA000)
        mDisplay->writeToDisplay(a_address, a_value);
    else if (a_address < 0xC000) {
        if (rtc.areRtcRegsSelected) {
            writeRTCRegister(a_value);
        } else
            mRam[ramBank][a_address - 0xA000] = a_value;
    } else if (a_address < 0xD000)
        mWRam[0][a_address - 0xC000] = a_value;
    else if (a_address < 0xE000)
        mWRam[wRamBank][a_address - 0xD000] = a_value;
    else if (a_address < 0xFE00)
        writeByte(a_address - 0x2000, a_value);
    else if (a_address < 0xFEA0)
        mDisplay->writeToDisplay(a_address, a_value);
    else if (a_address < 0xFF00)
        ;
    else if (a_address < 0xFF80)
        writeIO(a_address, a_value);
    else if (a_address < 0xFFFE)
        mHRam[a_address - 0xFF80] = a_value;
    else
        writeIO(a_address, a_value);
}

void MemoryMap::sendMBC1Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) {
        // if (mRomMode)
        //{
        romBank = a_value & 0x1F;
        if (romBank == 0) romBank++;
        /*}
        else
        {
            romBank = (romBank & 0xE0) | (a_value & 0x1F);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank ==
        0x60) romBank++;
        }*/
    } else if (a_address >= 0x4000 && a_address < 0x6000) {
        if (mRomMode) {
            romBank = (romBank & 0x1F) | ((a_value & 3) << 5);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60) romBank++;
        } else {
            ramBank = a_value & 3;
        }
    } else if (a_address >= 0x6000 && a_address < 0x8000)
        mRomMode = (a_value & 1) == 0;
}

void MemoryMap::sendMBC2Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) {
        // rom bank change
        romBank = a_value & 0xF;
        if (romBank == 0) romBank++;
    }
}

void MemoryMap::sendMBC3Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) romBank = a_value & 0x7F;
    else if (a_address >= 0x4000 && a_address < 0x6000) {
        switch (a_value) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
                ramBank = a_value;
                rtc.areRtcRegsSelected = false;
                break;
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
                rtc.rtcRegSelect = a_value;
                rtc.areRtcRegsSelected = true;
                break;
            default: LOG(a_value);
        }
    } else if (a_address >= 0x6000 && a_address < 0x8000) {
        if (rtc.latch == 0 && (a_value & 1) == 1) {
            rtc.dh = rtc2.dh;
            rtc.dl = rtc2.dl;
            rtc.hr = rtc2.hr;
            rtc.min = rtc2.min;
            rtc.sec = rtc2.sec;
            rtc.latch = 1;
        }
        rtc.latch = a_value & 1;
    }
}

void MemoryMap::sendMBC5Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x3000)  // 8 lower bits rom bank change
    {
        MBC5LowAddress = a_value;
        romBank = ((MBC5HighAddress << 8) | MBC5LowAddress) & 0x1FF;
    } else if (a_address >= 0x3000 && a_address < 0x4000)  // 9 bit of rom bank change
    {
        MBC5HighAddress = a_value & 1;
        romBank = ((MBC5HighAddress << 8) | MBC5LowAddress) & 0x1FF;
    } else if (a_address >= 0x4000 && a_address < 0x6000)  // ram bank change
        ramBank = a_value & 0xF;
}

void MemoryMap::writeRTCRegister(u8 a_value) {
    switch (rtc.rtcRegSelect) {
        case 0x8:
            rtc2.sec = a_value;
            rtc.sec = a_value;
            break;
        case 0x9:
            rtc2.min = a_value;
            rtc.min = a_value;
            break;
        case 0xA:
            rtc2.hr = a_value;
            rtc.hr = a_value;
            break;
        case 0xB:
            rtc2.dl = a_value;
            rtc.dl = a_value;
            break;
        case 0xC:
            rtc2.dh = a_value;
            rtc.dh = a_value;
            break;
        default: LOG(rtc.rtcRegSelect);
    }
}

void MemoryMap::writeIO(u16 a_address, u8 a_value) {
    if (a_address >= 0xFF10 && a_address < 0xFF40) {
        mSound->writeToSound(a_address, a_value);
        return;
    }
    switch (a_address) {
        case 0xFF00:                    // P1-Conotrols
            if ((a_value & 0x32) == 0)  // SGB Reset
            {
                ;
            } else
                mInput->writeRegister(a_value);
            break;
        case 0xFF01:  // SB-Serial Transfer data
            ST.trans = a_value;
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF02:  // SC-SIO Control
            ST.start = (a_value >> 7) & 1;
            ST.speed = (a_value >> 1) & 1;
            ST.cType = (a_value & 1);
            IOMap[a_address][0] = a_value;
            if (ST.start) {
#ifdef USE_SDL_NET
                if (net.isActive()) {
                    net.send(ST.trans);
                    ST.rec = (u8)net.recieve();
                    mem[0xFF01][0] = ST.rec;
                    mem[address][0] = val & 0x7F;
                    mem[0xFF0F][0] |= 8;
                } else
#endif
                {
                    if (ST.cType) {
                        IOMap[0xFF01][0] = 0xFF;
                        IOMap[a_address][0] = a_value & 0x7F;
                        IOMap[0xFF0F][0] |= 8;
                    }
                }
            }
            break;
        case 0xFF04:  // DIV-Divider Register
        case 0xFF05:  // TIMA Register
        case 0xFF06:  // TMA Register
        case 0xFF07:  // TAC Register
            mTimer->writeRegister(a_address, a_value);
            break;
        case 0xFF0F: writeIFRegister(a_value); break;  // IF Register
        case 0xFF46: DMATransfer(a_value); break;  // DMA
        case 0xFF40:  // LCDC
        case 0xFF41:  // STAT
        case 0xFF42:  // SCY
        case 0xFF43:  // SCX
        case 0xFF44:  // LY
        case 0xFF45:  // LYC
        case 0xFF47:  // BGP
        case 0xFF48:  // OBP0
        case 0xFF49:  // OBP1
        case 0xFF4A:  // WY
        case 0xFF4B:  // WX
        case 0xFF4F:  // VRAM bank
            mDisplay->writeToDisplay(a_address, a_value);
            break;
        case 0xFF4D: mPrepareSpeedChange = (a_value & 1) == 1; break;
        case 0xFF51: hdma.src = (hdma.src & 0xF0) | (a_value << 8); break;             // HDMA Source High
        case 0xFF52: hdma.src = (hdma.src & 0xFF00) | (a_value & 0xF0); break;         // HDMA Source Low
        case 0xFF53: hdma.dest = (hdma.dest & 0xFF) | ((a_value & 0x1F) << 8); break;  // HDMA Dest High
        case 0xFF54: hdma.dest = (hdma.dest & 0x1F00) | (a_value & 0xF0); break;       // HDMA Dest Low
        case 0xFF55:  // HDMA Transfer
            hdma.mode = ((a_value >> 7) & 1) != 0;
            hdma.length = ((a_value & 0x7F) + 1) * 0x10;
            if (hdma.active && !hdma.mode) {  //  Stop a current H-BLANK HDMA
                hdma.active = false;
                hdma.mode = false;
            } else if (!hdma.mode && !hdma.active) {  // Start a General HDMA
                HDMATransfer(hdma.src, hdma.dest, hdma.length);
                hdma.mode = true;
                hdma.length = 0x800;
            } else if(hdma.mode) {  // Start H-Blank HDMA
                hdma.active = true;
                hdma.mode = false;
            }
            break;
        case 0xFF68:
        case 0xFF69:
        case 0xFF6A:
        case 0xFF6B: mDisplay->writeToDisplay(a_address, a_value); break;
        case 0xFF70:
            IOMap[a_address][0] = a_value;
            if (mIsColor) {
                wRamBank = a_value & 7;
                if (wRamBank == 0) wRamBank++;
            }
            break;
        case 0xFFFF: IERegister = a_value; break;
        default: IOMap[a_address][0] = a_value;
    }
}

void MemoryMap::DMATransfer(u8 address) {
    for (int src = address << 8, dest = 0xFE00; dest < 0xFEA0; dest++, src++) {
        mDisplay->writeToDisplay(dest, readByte(src));
    }
}

void MemoryMap::HDMATransfer(u16 source, u16 dest, u32 length) {
    for (int i = 0; i < length; i++) {
        writeByte(0x8000 + dest + i, readByte(source + i));
    }
}

void MemoryMap::HBlankHDMA() {
    if (hdma.active) {
        for (int i = 0; i < 0x10; ++i) {
            writeByte(0x8000 + hdma.dest + i, readByte(hdma.src + i));
        }
        hdma.dest += 0x10;
        hdma.src += 0x10;
        hdma.length -= 0x10;

        if (hdma.length <= 0) {
            hdma.mode = true;
            hdma.length = 0x800;
            hdma.active = false;
        }
    }
}

void MemoryMap::rtcCounter(void) {
    if (((rtc2.dh >> 6) & 1) != 0) {
        return;
    }
    rtc2.sec++;
    if (rtc2.sec < 60) {
        return;
    }
    rtc2.sec = 0;
    rtc2.min++;
    if (rtc2.min < 60) {
        return;
    }
    rtc2.min = 0;
    rtc2.hr++;
    if (rtc2.hr < 24) {
        return;
    }
    rtc2.hr = 0;
    if (rtc2.dl < 0xFF) rtc2.dl++;
    else {
        if ((rtc2.dh & 1) == 0) rtc2.dh |= 1;
        else {
            rtc2.dh |= 0x80;
            rtc2.dh &= 0xFE;
            rtc.dl = 0;
        }
    }
}

void MemoryMap::save_sram() {
    std::string filename;
    std::ofstream save_file;
    std::ofstream rtc_file;

    auto first_index = mRomFilename.find_last_of('/') + 1;
    filename = mRomFilename.substr(first_index);
    auto last_index = filename.find_last_of('.');
    filename = filename.substr(0, last_index);
    std::cout << "Saving: " << filename << "\n";
    auto sav_filename = "savs/" + filename + ".sav";
    save_file.open(sav_filename, std::ios::binary);
    if (save_file.fail()) {
        std::cout << "WARNING: Couldn't save SRAM" << std::endl;
        return;
    }
    for (auto &ram_bank : mRam)
        save_file.write(reinterpret_cast<char *>(&ram_bank[0]), ram_bank.size());
    save_file.close();

    if (with_timer) {
        auto rtc_filename = "savs/" + filename + ".rtc";
        rtc_file.open(filename, std::ios::binary);
        if (rtc_file.fail()) {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtc_file.write((char *)&rtc, sizeof(RTC_Regs));
        rtc_file.write((char *)&rtc2, sizeof(RTC_Regs));
        rtc_file.close();
    }
}

void MemoryMap::load_sram() {
    std::string filename;
    std::ifstream save_file;
    std::ifstream rtc_file;

    auto first_index = mRomFilename.find_last_of('/') + 1;
    filename = mRomFilename.substr(first_index);
    auto last_index = filename.find_last_of('.');
    filename = filename.substr(0, last_index);

    std::cout << "Loading: " << filename << "\n";
    auto sav_filename = "savs/" + filename + ".sav";
    save_file.open(sav_filename, std::ios::binary);
    if (save_file.fail()) {
        std::cout << "WARNING: Couldn't load SRAM" << std::endl;
        return;
    }
    for (auto &ram_bank : mRam)
        save_file.read(reinterpret_cast<char *>(&ram_bank[0]), ram_bank.size());
    save_file.close();

    if (with_timer) {
        auto rtc_filename = "savs/" + filename + ".rtc";
        rtc_file.open(rtc_filename, std::ios::binary);
        if (rtc_file.fail()) {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtc_file.read((char *)&rtc, sizeof(RTC_Regs));
        rtc_file.read((char *)&rtc2, sizeof(RTC_Regs));
        rtc_file.close();
        // initRTCTimer();
    }
}

void MemoryMap::updateIO(int a_cycles) {
    mDisplay->update(a_cycles >> mCurrentSpeed);
    mSound->updateCycles(a_cycles);
    mTimer->update(a_cycles);
}

int MemoryMap::changeSpeed() {
    if (!mIsColor) {
        return 0;
    }
    if (mPrepareSpeedChange) {
        if (mCurrentSpeed == 0) mCurrentSpeed = 1;
        else
            mCurrentSpeed = 0;
        mPrepareSpeedChange = false;
    }
    return mCurrentSpeed;
}

u8 MemoryMap::getEnabledInterrupts() {
    return IERegister & readIFRegister();
}

void MemoryMap::resetInterruptRequest(int interrupt) {
    switch (interrupt) {
        case eInterrupts::VBLANK: mDisplay->mVBlankInterruptRequest = false; break;
        case eInterrupts::LCDC: mDisplay->mLCDInterruptRequest = false; break;
        case eInterrupts::TIMER: mTimer->InterruptBit = false; break;
        default: printf("Resseting other interrupt %d\n", interrupt); break;
    }
}

int MemoryMap::readIFRegister() {
    u8 vblank = mDisplay->mVBlankInterruptRequest ? eInterrupts::VBLANK : 0;
    u8 lcdc = mDisplay->mLCDInterruptRequest ? eInterrupts::LCDC : 0;
    u8 timer = mTimer->InterruptBit ? eInterrupts::TIMER : 0;
    u8 serial = 0;
    u8 joypad = 0;
    return serial | joypad | timer | vblank | lcdc;
}

void MemoryMap::writeIFRegister(u8 value) {
    mDisplay->mVBlankInterruptRequest = (value & eInterrupts::VBLANK) != 0;
    mDisplay->mLCDInterruptRequest = (value & eInterrupts::LCDC) != 0;
    mTimer->InterruptBit = (value & eInterrupts::TIMER) != 0;
    if ((value & eInterrupts::SERIAL) || (value & eInterrupts::JOYPAD)) {
        printf("Other interrupts request\n");
    }
}

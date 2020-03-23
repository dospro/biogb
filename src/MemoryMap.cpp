#include <iostream>
#include <fstream>
#include "MemoryMap.h"
#include "imp/video/cSDLDisplay.h"
#include "imp/audio/cSDLSound.h"
#include "Exceptions.h"


extern bool isColor;

MemoryMap::MemoryMap() :
        mCurrentSpeed{0},
        mDisplay{},
        mSound{},
        mInput{},
        mInterrupts{},
        mTimer{},
        romBank{1},
        ramBank{},
        wRamBank{1},
        mRomMode{},
        hi{},
        lo{},
        source{},
        dest{},
        hdma{},
        rtc{},
        rtc2{},
        ST{} {}

MemoryMap::~MemoryMap() = default;

bool MemoryMap::load_rom(const std::string& file_name)
{
    mRomFilename = std::string(file_name);
    RomLoader loader{file_name};
    mRom = loader.get_rom();
    isColor = loader.is_color();
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
    mDisplay = std::make_unique<cSDLDisplay>(isColor);
    std::cout << "OK" << std::endl;
    mDisplay->getMemoryPointer(this);

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
    mInterrupts = std::make_unique<cInterrupts>();
    mTimer = std::make_unique<cTimer>(mInterrupts.get());
}

u8 MemoryMap::readByte(u16 address)
{
    if (address < 0x8000)
        return readRom(address);
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
    else if (address < 0xFF00);
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
}

int MemoryMap::readIO(int a_address)
{
    switch (a_address)
    {
        case 0xFF04:
        case 0xFF05:
        case 0xFF06:
        case 0xFF07:
            return mTimer->readRegister(a_address);
        case 0xFF0F:
            return mInterrupts->readRegister(a_address);
        case 0xFF4D:
            return (static_cast<int>(mPrepareSpeedChange)) | (mCurrentSpeed << 7);
        case 0xFF40:  // LCDC
        case 0xFF41:  // STAT
        case 0xFF42:
        case 0xFF43:
        case 0xFF45:  // LYC
        case 0xFF4A:
        case 0xFF4B:
        case 0xFF68:
        case 0xFF69:
        case 0xFF6A:
        case 0xFF6B:
            return mDisplay->readFromDisplay(a_address);
        case 0xFFFF:
            return mInterrupts->readRegister(a_address);
        default:
            return IOMap[a_address][0];
    }
}

u8 MemoryMap::readRom(u16 a_address) const noexcept
{
    if (a_address < 0x4000)
        return mRom[0][a_address];
    else
        return mRom[romBank][a_address - 0x4000];
}

u8 MemoryMap::readRam(u16 address) const
{
    if (rtc.areRtcRegsSelected)
    {
        switch (rtc.rtcRegSelect)
        {
            case 0x8: return rtc.sec;
            case 0x9: return rtc2.min;
            case 0xA: return rtc2.hr;
            case 0xB: return rtc2.dl;
            case 0xC: return rtc2.dh;
            default: return mRam[ramBank][address - 0xA000];
        }
    }
    else
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
    else if (a_address < 0xFF00);
    else if (a_address < 0xFF80)
        writeIO(a_address, a_value);
    else if (a_address < 0xFFFE)
        mHRam[a_address - 0xFF80] = a_value;
    else
        writeIO(a_address, a_value);
}

void MemoryMap::sendMBC1Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x4000)
    {
        //if (mRomMode)
        //{
            romBank = a_value & 0x1F;
            if (romBank == 0)
                romBank++;
        /*}
        else
        {
            romBank = (romBank & 0xE0) | (a_value & 0x1F);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60)
                romBank++;
        }*/
    }
    else if (a_address >= 0x4000 && a_address < 0x6000)
    {
        if (mRomMode)
        {
            romBank = (romBank & 0x1F) | ((a_value & 3) << 5);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60)
                romBank++;
        }
        else
        {
            ramBank = a_value & 3;
        }
    }
    else if (a_address >= 0x6000 && a_address < 0x8000)
        mRomMode = (a_value & 1) == 0;
}

void MemoryMap::sendMBC2Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x4000)//rom bank change
    {
        romBank = a_value & 0xF;
        if (romBank == 0)
            romBank++;
    }
}

void MemoryMap::sendMBC3Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x4000)
        romBank = a_value & 0x7F;
    else if (a_address >= 0x4000 && a_address < 0x6000)
    {
        switch (a_value)
        {
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
            default:
                LOG(a_value);
        }
    }
    else if (a_address >= 0x6000 && a_address < 0x8000)
    {
        if (rtc.latch == 0 && (a_value & 1) == 1)
        {
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

void MemoryMap::sendMBC5Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x3000)//8 lower bits rom bank change
    {
        lo = a_value;
        romBank = ((hi << 8) | lo) & 0x1FF;
    }
    else if (a_address >= 0x3000 && a_address < 0x4000)//9 bit of rom bank change
    {
        hi = a_value & 1;
        romBank = ((hi << 8) | lo) & 0x1FF;
    }
    else if (a_address >= 0x4000 && a_address < 0x6000)//ram bank change
        ramBank = a_value & 0xF;
}

void MemoryMap::writeRTCRegister(u8 a_value)
{
    switch (rtc.rtcRegSelect)
    {
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
        default:
            LOG(rtc.rtcRegSelect);
    }
}

void MemoryMap::writeIO(u16 a_address, u8 a_value)
{
    if (a_address >= 0xFF10 && a_address < 0xFF40)
    {
        mSound->writeToSound(a_address, a_value);
        return;
    }
    switch (a_address)
    {
        case 0xFF00://P1-Conotrols
            if ((a_value & 0x32) == 0)//SGB Reset
            { ;
            }
            else
                mInput->writeRegister(a_value);
            break;
        case 0xFF01://SB-Serial Transfer data
            ST.trans = a_value;
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF02://SC-SIO Control
            ST.start = (a_value >> 7) & 1;
            ST.speed = (a_value >> 1) & 1;
            ST.cType = (a_value & 1);
            IOMap[a_address][0] = a_value;
            if (ST.start)
            {
#ifdef USE_SDL_NET
                if (net.isActive()) {
                        net.send(ST.trans);
                        ST.rec = (u8) net.recieve();
                        mem[0xFF01][0] = ST.rec;
                        mem[address][0] = val & 0x7F;
                        mem[0xFF0F][0] |= 8;
                    }
                    else
#endif
                {
                    if (ST.cType)
                    {
                        IOMap[0xFF01][0] = 0xFF;
                        IOMap[a_address][0] = a_value & 0x7F;
                        IOMap[0xFF0F][0] |= 8;
                    }
                }
            }
            break;
        case 0xFF04: // DIV-Divider Register
        case 0xFF05: // TIMA Register
        case 0xFF06: // TMA Register
        case 0xFF07: // TAC Register
            mTimer->writeRegister(a_address, a_value);
            break;
        case 0xFF0F: // IF Register
            mInterrupts->writeRegister(a_address, a_value);
            break;
//        case 0xFF41:
//            IOMap[a_address][0] = (IOMap[a_address][0] & 7) | (a_value & 0xF8); //Just write upper 5 bits
//            break;
        case 0xFF44://LY
            IOMap[a_address][0] = 0;
            break;

        case 0xFF46://DMA
            DMATransfer(a_value);
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF40: //LCDC
//        case 0xFF44: //LY
        case 0xFF41:  // STAT
        case 0xFF42:  // SCY
        case 0xFF43:  // SCX
        case 0xFF45:  // LYC
        case 0xFF47:  // BGP
        case 0xFF48:  // OBP0
        case 0xFF49:  // OBP1
        case 0xFF4A:  // WY
        case 0xFF4B:  // WX
        case 0xFF4F:  // VRAM bank
            mDisplay->writeToDisplay(a_address, a_value);
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF4D:
            mPrepareSpeedChange = (a_value & 1) == 1;
            break;
        case 0xFF55://HDMA Transfer
            hdma.mode = ((a_value >> 7) & 1) != 0;
            hdma.length = ((a_value & 0x7F) + 1) * 0x10;
            dest = ((IOMap[0xFF53][0] << 8) | IOMap[0xFF54][0]) & 0x1FF0;
            source = ((IOMap[0xFF51][0] << 8) | IOMap[0xFF52][0]) & 0xFFF0;
//            std::cout << "Write to FF55: " << "\n";
//            std::cout << "Mode: " << ((a_value >> 7) & 1) << "\n";
//            std::cout << "Length: " << std::hex << hdma.length << std::dec << "\n";
//            std::cout << "Source: " << std::hex << source << std::dec << "\n";
//            std::cout << "Destiny: " << std::hex << dest << std::dec << "\n";
            if (hdma.active && !hdma.mode)
            {
                //std::cout << "HDMA: Stop" << "\n";
                IOMap[a_address][0] |= 0x80;
                //std::cout << "FF55: " << std::hex << static_cast<int>(IOMap[a_address][0]) << std::dec << "\n";
                hdma.active = false;
                break;
            }
            else if (!hdma.mode && !hdma.active)
            {
                //std::cout << "HDMA: Mode 0 start" << "\n";
                HDMATransfer(source, dest, hdma.length);
                IOMap[a_address][0] = 0xFF;
                //std::cout << "FF55: " << std::hex << static_cast<int>(IOMap[a_address][0]) << std::dec << "\n";
                break;
            }
            else
            {
                //std::cout << "HDMA: Mode 1 active" << "\n";
                hdma.active = true;
                IOMap[a_address][0] = a_value & 0x7F;
                //std::cout << "FF55: " << std::hex << static_cast<int>(IOMap[a_address][0]) << std::dec << "\n";
            }

            break;
        case 0xFF68:
        case 0xFF69:
        case 0xFF6A:
        case 0xFF6B:
            mDisplay->writeToDisplay(a_address, a_value);
            break;
        case 0xFF70:
            IOMap[a_address][0] = a_value;
            if (isColor)
            {
                wRamBank = a_value & 7;
                if (wRamBank == 0)
                    wRamBank++;
            }
            break;
        case 0xFFFF:
            mInterrupts->writeRegister(a_address, a_value);
            break;
        default:
            IOMap[a_address][0] = a_value;
    }
}

void MemoryMap::DMATransfer(u8 address)
{
    int temp = (address << 8);
    for (int i = 0xFE00; i < 0xFEA0; i++)
    {
        IOMap[i][0] = readByte(temp);
        mDisplay->writeToDisplay(i, IOMap[i][0]);
        temp++;
    }
}

void MemoryMap::HDMATransfer(u16 source, u16 dest, u32 length)
{
    for (int i = 0; i < length; i++)
    {
        writeByte(0x8000 + dest + i, readByte(source + i));
    }
}

void MemoryMap::HBlankHDMA()
{
    if (hdma.active)
    {
        //std::cout << "HDMA at LY: " << static_cast<int>(IOMap[0xFF44][0]) << "\n";
        for (int i = 0; i < 0x10; ++i)
        {
            writeByte(0x8000 + dest + i, readByte(source + i));
        }
        dest += 0x10;
        IOMap[0xFF53][0] = dest >> 8;
        IOMap[0xFF54][0] = dest & 0xFF;
        source += 0x10;
        IOMap[0xFF51][0] = source >> 8;
        IOMap[0xFF52][0] = source & 0xFF;
        hdma.length -= 0x10;
        //std::cout << "Length after transfer: " << std::hex << static_cast<int>(hdma.length) << std::dec << "\n";


        if (hdma.length <= 0)
        {
            IOMap[0xFF55][0] = 0xFF;
            hdma.active = false;
        }
        else
        {
            IOMap[0xFF55][0] = hdma.length / 0x10 - 1;
        }
        //std::cout << "FF55 length: " << std::hex << static_cast<int>(IOMap[0xFF55][0]) << std::dec << "\n";
    }
}

void MemoryMap::rtcCounter(void)
{
    if (((rtc2.dh >> 6) & 1) != 0)
    {
        return;
    }
    rtc2.sec++;
    if (rtc2.sec < 60)
    {
        return;
    }
    rtc2.sec = 0;
    rtc2.min++;
    if (rtc2.min < 60)
    {
        return;
    }
    rtc2.min = 0;
    rtc2.hr++;
    if (rtc2.hr < 24)
    {
        return;
    }
    rtc2.hr = 0;
    if (rtc2.dl < 0xFF)
        rtc2.dl++;
    else
    {
        if ((rtc2.dh & 1) == 0)
            rtc2.dh |= 1;
        else
        {
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
        save_file.write(reinterpret_cast<char *> (&ram_bank[0]), ram_bank.size());
    save_file.close();

    if (with_timer) {
        auto rtc_filename = "savs/" + filename + ".rtc";
        rtc_file.open(filename, std::ios::binary);
        if (rtc_file.fail()) {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtc_file.write((char *) &rtc, sizeof(RTC_Regs));
        rtc_file.write((char *) &rtc2, sizeof(RTC_Regs));
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
    for (auto & ram_bank : mRam)
        save_file.read(reinterpret_cast<char *>(&ram_bank[0]), ram_bank.size());
    save_file.close();

    if (with_timer) {
        auto rtc_filename = "savs/" + filename + ".rtc";
        rtc_file.open(rtc_filename, std::ios::binary);
        if (rtc_file.fail()) {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtc_file.read((char *) &rtc, sizeof(RTC_Regs));
        rtc_file.read((char *) &rtc2, sizeof(RTC_Regs));
        rtc_file.close();
        //initRTCTimer();
    }
}

void MemoryMap::updateIO(int a_cycles)
{
    mSound->updateCycles(a_cycles >> mCurrentSpeed);
    mTimer->update(a_cycles >> mCurrentSpeed);
}

int MemoryMap::changeSpeed()
{
    if (mPrepareSpeedChange)
    {
        if (mCurrentSpeed == 0)
            mCurrentSpeed = 1;
        else
            mCurrentSpeed = 0;
        mPrepareSpeedChange = false;
    }
    return mCurrentSpeed;
}

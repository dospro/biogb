//
// Created by dospro on 25/11/15.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include "cMemory.h"
#include "tables.h"
#include "imp/video/cSDLDisplay.h"
#include "imp/audio/cPortAudio.h"


extern u8 jpb, jpd; //Joy pad buttons and directions
extern bool isColor;
extern bool speedChange;
extern u32 currentSpeed;

cMemory::cMemory()
{
    mDisplay = nullptr;
}

cMemory::~cMemory()
{
    if (mDisplay != nullptr)
    {
        delete (mDisplay);
        mDisplay = nullptr;
    }
}

bool cMemory::loadRom(const char *fileName)
{
    std::ifstream file;
    int size, banks;
    mRomFilename = std::string{fileName};
    file.open(fileName, std::ios::binary);
    if (file.fail())
    {
        std::cout << "Couldn't load file: " << fileName << std::endl;
        return false;
    }
    info.color = 0;
    info.mbc = 0;
    info.ramSize = 0;
    info.romSize = 0;
    size = 0;
    banks = 0;

    file.seekg(0x134);
    file.read(info.name, 15);
    file.seekg(0x143);
    file.read((char *) (&info.color), 1);
    file.seekg(0x147);
    file.read((char *) &info.mbc, 1);
    file.seekg(0x148);
    file.read((char *) &info.romSize, 1);
    file.seekg(0x149);
    file.read((char *) &info.ramSize, 1);

    for (int i = 0; i < 12; i++)
    {
        if (romSizeList[i].id == info.romSize)
        {
            size = romSizeList[i].size;
            banks = romSizeList[i].banks;
            break;
        }
    }
    if (size == 0 || banks == 0)
    {
        file.seekg(0, std::ios::end);
        size = file.tellg();
        banks = size / 0x4000;
        std::cout << "WARNING: Unknown chip found. Trying generic emulation."
        << std::endl;
    }

    file.seekg(0);

    for (int i = 0; i < banks; i++)
    {
        mRom.push_back(std::array<u8, 0x4000> {});
        file.read(reinterpret_cast<char *>(&mRom[i][0]), 0x4000);
        for (int j = 0; j < 0x4000; j++)
            IOMap[j][i] = mRom[i][j];
    }

    file.close();

    for (int i = 0; i < 5; i++)
    {
        if (ramSizeList[i].id == info.ramSize)
        {
            banks = ramSizeList[i].banks;
            std::cout << "Ram Banks: " << banks << "\n";
            break;
        }
    }

    for (int i = 0; i < banks; ++i)
    {
        mRam.emplace_back(std::array<u8, 0x2000>{});
    }


    if (info.color == 0x80 || info.color == 0xC0)
        isColor = true;
    else
        isColor = false;

    mWRam.push_back(std::array<u8, 0x1000>{});
    mWRam.push_back(std::array<u8, 0x1000>{});
    if (isColor)
    {
        for (int i = 0; i < 6; ++i)
        {
            mWRam.push_back(std::array<u8, 0x1000>{});
        }
    }
    loadSram();
    romBank = 1;
    ramBank = 0;
    vRamBank = 0;
    wRamBank = 1;
    mm = 0;
    hi = lo = 0;
    source = 0;
    dest = 0;
    std::memset(&hdma, 0, sizeof(HDMA));
    std::memset(&rtc, 0, sizeof(RTC_Regs));
    std::memset(&rtc2, 0, sizeof(RTC_Regs));
    std::memset(&ST, 0, sizeof(SerialTransfer));
    //TODO: Refactorizar esto
    std::cout << "Display.";
    try
    {
        mDisplay = new cSDLDisplay(isColor);
        // TODO: Change exception type
    } catch (std::exception e)
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << "OK" << std::endl;
    mDisplay->getMemoryPointer(this);

    std::cout << "Sound...";
    mSound = new cPortAudio;
    if (!mSound)
    {
        std::cout << "No memory for Dound object" << std::endl;
        return false;
    }
    if (!mSound->init(44100, 8, 512))
    {
        std::cout << "Sound was not inited." << std::endl;
        return false;
    }
    std::cout << "OK" << std::endl;
    std::cout << "Turning sound on" << std::endl;
    mSound->turnOn();


    return true;
}

u8 cMemory::readByte(u16 address)
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
    else if (address < 0xFF10)
        return IOMap[address][0];
    else if (address < 0xFF40)
        return mSound->readFromSound(address);
    else if (address < 0xFF80)
        return IOMap[address][0];
    else if (address < 0xFFFE)
        return mHRam[address - 0xFF80];
    else
        return IOMap[address][0];
}

u8 cMemory::readRom(u16 a_address) const noexcept
{
    if (a_address < 0x4000)
        return mRom[0][a_address];
    else
        return mRom[romBank][a_address - 0x4000];
}

u8 cMemory::readRam(u16 address) const
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

void cMemory::writeByte(u16 a_address, u8 a_value)
{
    if (a_address < 0x8000)
    {
        if (isMBC1(info))
        {
            sendMBC1Command(a_address, a_value);
        }
        else if (isMBC2(info))
        {
            sendMBC2Command(a_address, a_value);
        }
        else if (isMBC3(info))
        {
            sendMBC3Command(a_address, a_value);
        }
        else if (isMBC5(info))
        {
            sendMBC5Command(a_address, a_value);
        }
    }
    else if (a_address < 0xA000)
        mDisplay->writeToDisplay(a_address, a_value);
    else if (a_address < 0xC000)
    {
        if (rtc.areRtcRegsSelected)
        {
            writeRTCRegister(a_value);
        }
        else
            mRam[ramBank][a_address - 0xA000] = a_value;
    }
    else if (a_address < 0xD000)
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

bool cMemory::isMBC1(gbHeader a_header) const
{
    return (a_header.mbc == 1 || a_header.mbc == 2 || a_header.mbc == 3);
}

bool cMemory::isMBC2(gbHeader a_header) const
{
    return (a_header.mbc == 5 || a_header.mbc == 6);
}

bool cMemory::isMBC3(gbHeader a_header) const
{
    return a_header.mbc == 0x0F || a_header.mbc == 0x10 || a_header.mbc == 0x11 || a_header.mbc == 0x12 ||
           a_header.mbc == 0x13;
}

bool cMemory::isMBC5(gbHeader a_header) const
{
    return a_header.mbc == 0x19 || a_header.mbc == 0x1A || a_header.mbc == 0x1B || a_header.mbc == 0x1C ||
           a_header.mbc == 0x1D ||
           a_header.mbc == 0x1E;
}

void cMemory::sendMBC1Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x4000)
    {
        if (mm == 1)
        {
            romBank = a_value & 0x1F;
            if (romBank == 0)
                romBank++;
        }
        else
        {
            romBank = (romBank & 0xE0) | (a_value & 0x1F);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60)
                romBank++;
        }
    }
    else if (a_address >= 0x4000 && a_address < 0x6000)
    {
        if (mm == 1)
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
        mm = a_value & 1;
}

void cMemory::sendMBC2Command(u16 a_address, u8 a_value)
{
    if (a_address >= 0x2000 && a_address < 0x4000)//rom bank change
    {
        romBank = a_value & 0xF;
        if (romBank == 0)
            romBank++;
    }
}

void cMemory::sendMBC3Command(u16 a_address, u8 a_value)
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

void cMemory::sendMBC5Command(u16 a_address, u8 a_value)
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

void cMemory::writeRTCRegister(u8 a_value)
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

void cMemory::writeIO(u16 a_address, u8 a_value)
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
            else if ((a_value & 16) == 0)//Directions
                IOMap[a_address][0] = ((jpd ^ 255) & 0xF) | 0xE0;
            else if ((a_value & 32) == 0)//Buttons
                IOMap[a_address][0] = ((jpb ^ 255) & 0xF) | 0xD0;
            else
                IOMap[a_address][0] = 0xFF;
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
        case 0xFF04://DIV-Divider Register
            IOMap[a_address][0] = 0;
            break;
        case 0xFF41:
            IOMap[a_address][0] = (IOMap[a_address][0] & 7) | (a_value & 0xF8); //Just write upper 5 bits
            break;
        case 0xFF44://LY
            IOMap[a_address][0] = 0;
            break;
        case 0xFF46://DMA
            DMATransfer(a_value);
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF47://BGP
        case 0xFF48://OBP0
        case 0xFF49://OBP1
        case 0xFF4F:// VRAM bank
            mDisplay->writeToDisplay(a_address, a_value);
            IOMap[a_address][0] = a_value;
            break;
        case 0xFF4D:
            speedChange = a_value & 1;
            IOMap[a_address][0] = ((int) speedChange) | (currentSpeed << 7);
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
            /* TODO: Display ya maneja todo internamente, para quitar mem, falta modificar readByte para leer desde display */
        case 0xFF68:
            IOMap[a_address][0] = a_value;
            mDisplay->writeToDisplay(a_address, a_value);
            break;
        case 0xFF69:
            IOMap[a_address][0] = a_value;
            mDisplay->writeToDisplay(a_address, a_value);
            if (IOMap[0xFF68][0] & 0x80)//Autoincrement
                IOMap[0xFF68][0]++;
            break;
        case 0xFF6A:
            IOMap[a_address][0] = a_value;
            mDisplay->writeToDisplay(a_address, a_value);
            break;
        case 0xFF6B:
            IOMap[a_address][0] = a_value;
            mDisplay->writeToDisplay(a_address, a_value);
            if (IOMap[0xFF6A][0] & 0x80)
                IOMap[0xFF6A][0]++;
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
        default:
            IOMap[a_address][0] = a_value;
    }
}

void cMemory::DMATransfer(u8 address)
{
    int i;
    int temp = (address << 8);
    for (i = 0xFE00; i < 0xFEA0; i++)
    {
        IOMap[i][0] = readByte(temp);
        mDisplay->writeToDisplay(i, IOMap[i][0]);
        temp++;
    }
}

void cMemory::HDMATransfer(u16 source, u16 dest, u32 length)
{
    for (int i = 0; i < length; i++)
    {
        writeByte(0x8000 + dest + i, readByte(source + i));
    }
}

void cMemory::HBlankHDMA()
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

void cMemory::rtcCounter(void)
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

void cMemory::hBlankDraw()
{
    mDisplay->hBlankDraw();
}

void cMemory::updateScreen()
{
    mDisplay->updateScreen();
}

void cMemory::saveSram()
{
    std::string fileName;
    std::ofstream saveFile;
    std::ofstream rtcFile;
#ifndef LINUX
    fileName = "savs\\";
#else
    fileName = "savs/";
#endif
    auto lastIndex = mRomFilename.find_last_of("/") + 1;
    auto temp = mRomFilename.substr(lastIndex);
    fileName += temp.substr(0, temp.find_last_of("."));
    fileName += ".sav";
    std::cout << "Saving: " << fileName << "\n";
    saveFile.open(fileName, std::ios::binary);
    if (saveFile.fail())
    {
        std::cout << "WARNING: Couldn't save SRAM" << std::endl;
        return;
    }
    for (int i = 0; i < mRam.size(); ++i)
        saveFile.write(reinterpret_cast<char *> (&mRam[i][0]), mRam[i].size());

    saveFile.close();

    if (info.mbc == 0xF || info.mbc == 0x10)
    {
#ifndef LINUX
        fileName = "savs\\";
#else
        fileName = "savs/";
#endif
        fileName += info.name;
        fileName += ".rtc";
        rtcFile.open(fileName, std::ios::binary);
        if (rtcFile.fail())
        {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtcFile.write((char *) &rtc, sizeof(RTC_Regs));
        rtcFile.write((char *) &rtc2, sizeof(RTC_Regs));
        rtcFile.close();
    }
}

void cMemory::loadSram()
{
    std::string fileName;
    std::ifstream saveFile;
    std::ifstream rtcFile;
#ifndef LINUX
    fileName = "savs\\";
#else
    fileName = "savs/";
#endif
    auto lastIndex = mRomFilename.find_last_of("/") + 1;
    std::string temp = mRomFilename.substr(lastIndex);
    fileName += temp.substr(0, temp.find_last_of("."));
    fileName += ".sav";
    std::cout << "Loading: " << fileName << "\n";
    saveFile.open(fileName, std::ios::binary);
    if (saveFile.fail())
    {
        std::cout << "WARNING: Couldn't load SRAM" << std::endl;
        return;
    }
    for (int i = 0; i < mRam.size(); ++i)
        saveFile.read(reinterpret_cast<char *>(&mRam[i][0]), mRam[i].size());
    saveFile.close();

    if (info.mbc == 0xF || info.mbc == 0x10)
    {
#ifndef LINUX
        fileName = "savs\\";
#else
        fileName = "savs/";
#endif
        fileName += info.name;
        fileName += ".rtc";
        rtcFile.open(fileName.c_str(), std::ios::binary);
        if (rtcFile.fail())
        {
            std::cout << "WARNING: Couldn't save RTC file" << std::endl;
            return;
        }

        rtcFile.read((char *) &rtc, sizeof(RTC_Regs));
        rtcFile.read((char *) &rtc2, sizeof(RTC_Regs));
        rtcFile.close();
        //initRTCTimer();
    }
}
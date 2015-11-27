//
// Created by dospro on 25/11/15.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include "cMemory.h"
#include "tables.h"
#include "cSound.h"

extern cSound *sound;
extern u8 jpb, jpd; //Joy pad buttons and directions
extern bool isColor;
extern u32 BWColors[2][2];
extern u32 BGColors[64];
extern u32 OBJColors[64];
extern u32 BGPTable[2][2]; //cGfx.cpp
extern u32 WPTable[2][2]; //cGfx.cpp
extern u32 OBP0Table[2][2]; //cGfx.cpp
extern u32 OBP1Table[2][2]; //cGfx.cpp

extern bool speedChange;
extern u32 currentSpeed;

bool cMemory::loadRom(const char *fileName)
{
    std::ifstream file;
    int size, banks, i, j;

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

    for (i = 0; i < 12; i++)
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

    for (i = 0; i < banks; i++)
    {
        u8 tempBank[0x4000];
        file.read((char *) tempBank, 0x4000);
        for (j = 0; j < 0x4000; j++)
            mem[j][i] = tempBank[j];
    }

    file.close();

    if (info.color == 0x80 || info.color == 0xC0)
        isColor = true;
    else
        isColor = false;
    romBank = 1;
    ramBank = 1;
    vRamBank = 0;
    wRamBank = 1;
    mm = 0;
    hi = lo = 0;
    source = 0;
    dest = 0;
    ramEnable = false;
    std::memset(&hdma, 0, sizeof(HDMA));
    std::memset(&rtc, 0, sizeof(RTC_Regs));
    std::memset(&rtc2, 0, sizeof(RTC_Regs));
    std::memset(&ST, 0, sizeof(SerialTransfer));
    return true;
}

u8 cMemory::readByte(u16 address)
{
    if (address < 0x8000)
        return readRom(address);
    else if (address < 0xA000)
        return mem[address][vRamBank];
    else if (address < 0xC000)
    {
        return readRTCRegisters(address);
    }
    else if (address >= 0xD000 && address < 0xE000)
        return mem[address][wRamBank];
    else
        return mem[address][0];
}

u8 cMemory::readRom(u16 address) const
{
    if (address < 0x4000)
        return mem[address][0];
    else
        return mem[address - 0x4000][romBank];
}

u8 cMemory::readRTCRegisters(u16 address) const
{
    if (rtc.areRtcRegsSelected)
    {
        switch (rtc.rtcRegSelect)
        {
            case 0x8:
                return rtc.sec;
                break;
            case 0x9:
                return rtc2.min;
                break;
            case 0xA:
                return rtc2.hr;
                break;
            case 0xB:
                return rtc2.dl;
                break;
            case 0xC:
                return rtc2.dh;
                break;
            default:
                return mem[address][ramBank];
                LOG(rtc.rtcRegSelect);
        }
    }
    else
    {
        return mem[address][ramBank];
    }
}

void cMemory::writeByte(u16 address, u8 val)
{
    if (address < 0x8000)
    {
        if (isMBC1(info))
        {
            sendMBC1Command(address, val);
        }
        else if (isMBC2(info))
        {
            sendMBC2Command(address, val);
        }
        else if (isMBC3(info))
        {
            sendMBC3Command(address, val);
        }
        else if (isMBC5(info))
        {
            sendMBC5Command(address, val);
        }
    }
    else if (address < 0xFF00)
    {
        if (address < 0xA000)
            mem[address][vRamBank] = val;
        else if (address < 0xC000)
        {
            if (rtc.areRtcRegsSelected)
            {
                writeRTCRegister(val);
            }
            else
                mem[address][ramBank] = val;
        }
        else if (address >= 0xD000 && address < 0xE000)
            mem[address][wRamBank] = val;
        else
            mem[address][0] = val;
    }
    else
    {
        writeIO(address, val);
    }
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
    return a_header.mbc == 0x19 || a_header.mbc == 0x1A || a_header.mbc == 0x1B || a_header.mbc == 0x1C || a_header.mbc == 0x1D ||
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
    switch (a_address)
    {
        case 0xFF00://P1-Conotrols
            if ((a_value & 0x32) == 0)//SGB Reset
            { ;
            }
            else if ((a_value & 16) == 0)//Directions
                mem[a_address][0] = ((jpd ^ 255) & 0xF) | 0xE0;
            else if ((a_value & 32) == 0)//Buttons
                mem[a_address][0] = ((jpb ^ 255) & 0xF) | 0xD0;
            else
                mem[a_address][0] = 0xFF;
            break;
        case 0xFF01://SB-Serial Transfer data
            ST.trans = a_value;
            mem[a_address][0] = a_value;
            break;
        case 0xFF02://SC-SIO Control
            ST.start = (a_value >> 7) & 1;
            ST.speed = (a_value >> 1) & 1;
            ST.cType = (a_value & 1);
            mem[a_address][0] = a_value;
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
                        mem[0xFF01][0] = 0xFF;
                        mem[a_address][0] = a_value & 0x7F;
                        mem[0xFF0F][0] |= 8;
                    }
                }
            }
            break;
        case 0xFF04://DIV-Divider Register
            mem[a_address][0] = 0;
            break;
        case 0xFF10://NR10
        case 0xFF11://NR11
        case 0xFF12://NR12
        case 0xFF13://NR13
        case 0xFF14://NR14
        case 0xFF16://NR21
        case 0xFF17://NR22
        case 0xFF18://NR23
        case 0xFF19://NR24
        case 0xFF1A://NR30
        case 0xFF1B://NR31
        case 0xFF1C://NR32
        case 0xFF1D://NR33
        case 0xFF1E://NR34
        case 0xFF20://NR41
        case 0xFF21://NR42
        case 0xFF22://NR43
        case 0xFF23://NR44
        case 0xFF24://NR50
        case 0xFF25://NR51
        case 0xFF26://NR52
            mem[a_address][0] = sound->getSoundMessage(a_address, a_value);
            break;
        case 0xFF41:
            mem[a_address][0] = (mem[a_address][0] & 7) | (a_value & 0xF8); //Just write upper 5 bits
            break;
        case 0xFF44://LY
            mem[a_address][0] = 0;
            break;
        case 0xFF46://DMA
            DMATransfer(a_value);
            mem[a_address][0] = a_value;
            break;
        case 0xFF47://BGP
            BGPTable[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            BGPTable[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            BGPTable[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            BGPTable[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            mem[a_address][0] = a_value;
            break;
        case 0xFF48://OBP0
            OBP0Table[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            OBP0Table[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            OBP0Table[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            OBP0Table[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            mem[a_address][0] = a_value;
            break;
        case 0xFF49://OBP1
            OBP1Table[1][1] = BWColors[(a_value >> 7)][(a_value >> 6) & 1];
            OBP1Table[1][0] = BWColors[(a_value >> 5) & 1][(a_value >> 4) & 1];
            OBP1Table[0][1] = BWColors[(a_value >> 3) & 1][(a_value >> 2) & 1];
            OBP1Table[0][0] = BWColors[(a_value >> 1) & 1][(a_value & 1)];
            mem[a_address][0] = a_value;
            break;
        case 0xFF4D:
            speedChange = a_value & 1;
            mem[a_address][0] = ((int) speedChange) | (currentSpeed << 7);
            break;
        case 0xFF4F:
            if (isColor)
            {
                vRamBank = a_value & 1;
                mem[a_address][0] = vRamBank & 1;
            }
            else
                mem[a_address][0] = a_value;
            break;
        case 0xFF55://HDMA Transfer
            hdma.mode = (a_value >> 7) & 1;
            hdma.length = ((a_value & 0x7F) + 1) << 4;
            dest = ((mem[0xFF53][0] << 8) | mem[0xFF54][0]) & 0x1FF0;
            source = ((mem[0xFF51][0] << 8) | mem[0xFF52][0]) & 0xFFF0;
            if (hdma.active && !hdma.mode)
            {
                mem[a_address][0] = mem[a_address][0] | 0x80;
                hdma.active = false;
                break;
            }
            else if (!hdma.mode && !hdma.active)
                HDMATransfer(source, dest, hdma.length);
            else
                hdma.active = true;
            mem[a_address][0] = a_value & 0x7F;
            break;
        case 0xFF69:
            mem[a_address][0] = a_value;
            BGColors[mem[0xFF68][0] & 0x3F] = a_value;
            if (mem[0xFF68][0] & 0x80)//Autoincrement
                mem[0xFF68][0]++;
            break;
        case 0xFF6B:
            mem[a_address][0] = a_value;
            OBJColors[mem[0xFF6A][0] & 0x3F] = a_value;
            if (mem[0xFF6A][0] & 0x80)
                mem[0xFF6A][0]++;
            break;
        case 0xFF70:
            mem[a_address][0] = a_value;
            wRamBank = a_value & 7;
            if (wRamBank == 0)
                wRamBank++;
            break;
        default:
            mem[a_address][0] = a_value;
    }
}

void cMemory::DMATransfer(u8 address)
{
    int i;
    int temp = (address << 8);
    for (i = 0xFE00; i < 0xFEA0; i++)
    {
        mem[i][0] = readByte(temp);
        temp++;
    }
}

void cMemory::HDMATransfer(u16 source, u16 dest, u32 length)
{
    u32 i;
    for (i = 0; i < length; i++)
    {
        writeByte(0x8000 + dest + i, readByte(source + i));
    }
    mem[0xFF55][0] = 0xFF;
}

void cMemory::HBlankHDMA(void)
{

    int i;
    if (hdma.active)
    {
        for (i = 0; i < 0x10; i++)
        {
            writeByte(0x8000 + dest + i, readByte(source + i));
        }
        dest += 0x10;
        source += 0x10;
        hdma.length -= 0x10;

        if (hdma.length < 0x10)
        {
            mem[0xFF55][0] = 0xFF;
            hdma.active = false;
        }
        else
            mem[0xFF55][0]--;
    }
}

void cMemory::rtcCounter(void)
{
    if (((rtc2.dh >> 6) & 1) == 0)
    {
        rtc2.sec++;
        if (rtc2.sec >= 60)
        {
            rtc2.sec = 0;
            rtc2.min++;
            if (rtc2.min >= 60)
            {
                rtc2.min = 0;
                rtc2.hr++;
                if (rtc2.hr >= 24)
                {
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
            }
        }
    }
}
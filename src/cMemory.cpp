//
// Created by dospro on 25/11/15.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include "cMemory.h"
#include"tables.h"

extern bool isColor;
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
        std::cout << "WARNING: Uknown chip found. Trying generic emulation."
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
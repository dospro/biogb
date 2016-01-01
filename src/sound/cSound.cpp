/*
 *     Proyect: BioGB
 *    Filename: cSound.cpp
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rubén Daniel Gutiérrez Cruz <dospro@gmail.com>
 *        Date: 07-1-2007
 *
 *
 *	This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include"cSound.h"

cSound::cSound()
{
    buffer = NULL;
}

cSound::~cSound()
{
    if (buffer != NULL)
    {
        delete buffer;
        buffer = NULL;
    }
}

bool cSound::init(u32 a_frequency, u32 size, u32 bSize)
{
    soundActive = true;

    mChannel1 = new cSoundChannel1(a_frequency);
    mChannel2 = new cSoundChannel2(a_frequency);
    mChannel3 = new cSoundChannel3(a_frequency);
    mChannel4 = new cSoundChannel4(a_frequency);

    bufferSize = bSize;
    buffer = new u8[bufferSize];
    if (!buffer)
        return false;

    return true;
}

u8 cSound::readFromSound(u16 a_address)
{
    if (a_address < 0xFF16)
        return mChannel1->readRegister(a_address);
    else if (a_address < 0xFF1A)
        return mChannel2->readRegister(a_address);
    else if (a_address < 0xFF20)
        return mChannel3->readRegister(a_address);
    else if (a_address < 0xFF24)
        return mChannel4->readRegister(a_address);
    else if (a_address < 0xFF30)
    {
        switch (a_address)
        {
            case 0xFF24: //NR 50
                return NR50;
            case 0xFF25:
                return NR51;
            case 0xFF26: //NR 52
                return static_cast<u8>(mChannel1->getOnOffBit()
                                       | mChannel2->getOnOffBit()
                                       | mChannel3->getOnOffBit()
                                       | mChannel4->getOnOffBit()
                                       | (soundActive << 7)
                                       | 0x70);
        }
    }
    else
        return mChannel3->readRegister(a_address);
}

void cSound::writeToSound(u16 address, u8 value)
{
    switch (address)
    {
        case 0xFF10: //NR 10
        case 0xFF11: //NR 11
        case 0xFF12: //NR 12
        case 0xFF13: //NR 13
        case 0xFF14: //NR 14
            mChannel1->writeRegister(address, value);
        case 0xFF16: //NR 21
        case 0xFF17: //NR 22
        case 0xFF18: //NR 23
        case 0xFF19: //NR 24
            mChannel2->writeRegister(address, value);
        case 0xFF1A: //NR 30
        case 0xFF1B: //NR 31
        case 0xFF1C: //NR 32
        case 0xFF1D: //NR 33
        case 0xFF1E: //NR 34
            mChannel3->writeRegister(address, value);
        case 0xFF20: //NR 41
        case 0xFF21: //NR 42
        case 0xFF22: //NR 43
        case 0xFF23: //NR 44
            mChannel4->writeRegister(address, value);
        case 0xFF24: //NR 50
            NR50 = value;
            break;
        case 0xFF25:
            mChannel1->setOutputTerminal((value & 1) != 0, (value & 0x10) != 0);
            mChannel2->setOutputTerminal((value & 2) != 0, (value & 0x20) != 0);
            mChannel3->setOutputTerminal((value & 4) != 0, (value & 0x40) != 0);
            mChannel4->setOutputTerminal((value & 8) != 0, (value & 0x80) != 0);
            NR51 = value;
            break;
        case 0xFF26: //NR 52
            soundActive = (value >> 7) & 1;
            break;
        default:
            if (address >= 0xFF30 && address < 0xFF40)
            {
                mChannel3->writeRegister(address, value);
            }
            break;
    }
}

void cSound::fillBuffer(void)
{
    int sample1, sample2, sample3, sample4;
    int finalSample;

    for (int i = 0; i < bufferSize; ++i)
    {
        sample1 = 0;
        sample2 = 0;
        sample3 = 0;
        sample4 = 0;
        if (soundActive)
        {
            if (mChannel1->isOn())
                sample1 = mChannel1->getSample();
            if (mChannel2->isOn())
                sample2 = mChannel2->getSample();
            if (mChannel3->isOn())
                sample3 = mChannel3->getSample();
            if (mChannel4->isOn())
                sample4 = mChannel4->getSample();
        }
        sample1 -= 128;
        sample2 -= 128;
        sample3 -= 128;
        sample4 -= 128;

        finalSample = (sample1 + sample2 + sample3 + sample4) >> 4;

        buffer[i] = (finalSample + 128) & 0xFF;

    }
}

void cSound::updateCycles(s32 cycles)
{
    mChannel1->update(cycles);
    mChannel2->update(cycles);
    mChannel3->update(cycles);
    mChannel4->update(cycles);
}


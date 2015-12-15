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
#include"cCpu.h"

int divRatioFreq[] = {
        1048576,
        524288,
        262144,
        174753,
        131072,
        104858,
        87381,
        74898,
};

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
    generalFreq = a_frequency;
    soundActive = true;
    ch[1].counter1 = 0;
    ch[1].counter2 = 0;
    ch[1].onOff = false;
    ch[1].wavePattern = 2;
    ch[3].counter1 = 0;
    ch[3].counter2 = 0;
    ch[3].onOff = false;

    mChannel1 = new cSoundChannel1(a_frequency);
    mChannel3 = new cSoundChannel3(a_frequency);

    bufferSize = bSize;
    buffer = new u8[bufferSize];
    if (!buffer)
        return false;

    wavePatternDuty[0][0] = 0;
    wavePatternDuty[0][1] = 0xFF;
    wavePatternDuty[0][2] = 0xFF;
    wavePatternDuty[0][3] = 0xFF;
    wavePatternDuty[0][4] = 0xFF;
    wavePatternDuty[0][5] = 0xFF;
    wavePatternDuty[0][6] = 0xFF;
    wavePatternDuty[0][7] = 0xFF;


    wavePatternDuty[1][0] = 0;
    wavePatternDuty[1][1] = 0;
    wavePatternDuty[1][2] = 0xFF;
    wavePatternDuty[1][3] = 0xFF;
    wavePatternDuty[1][4] = 0xFF;
    wavePatternDuty[1][5] = 0xFF;
    wavePatternDuty[1][6] = 0xFF;
    wavePatternDuty[1][7] = 0xFF;

    wavePatternDuty[2][0] = 0;
    wavePatternDuty[2][1] = 0;
    wavePatternDuty[2][2] = 0;
    wavePatternDuty[2][3] = 0;
    wavePatternDuty[2][4] = 0xFF;
    wavePatternDuty[2][5] = 0xFF;
    wavePatternDuty[2][6] = 0xFF;
    wavePatternDuty[2][7] = 0xFF;

    wavePatternDuty[3][0] = 0;
    wavePatternDuty[3][1] = 0;
    wavePatternDuty[3][2] = 0;
    wavePatternDuty[3][3] = 0;
    wavePatternDuty[3][4] = 0;
    wavePatternDuty[3][5] = 0;
    wavePatternDuty[3][6] = 0xFF;
    wavePatternDuty[3][7] = 0xFF;

    for (int i = 0; i < 0x8000; ++i)
        noise[i] = (rand() * 100) % 256;

    return true;
}

u8 cSound::getCurrentSample(float freq, float &c1, u8 *pattern, s32 size, s32 &c2)
{
    float rsize = generalFreq / freq / size;

    if (c1 <= rsize)
    {
        ++c1;
        return pattern[c2];
    }
    else
    {
        ++c2;
        if (c2 >= size)
            c2 = 0;
        c1 -= rsize - 1;
        return pattern[c2];
    }
}

u8 cSound::getCurrentNoise(float freq, float *c1, u8 *pattern, s32 size, s32 *c2)
{
    float rsize = 4194304 / freq / size;

    if ((*c1) <= rsize)
    {
        ++(*c1);
        return pattern[*c2];
    }
    else
    {
        ++(*c2);
        if ((*c2) >= size)
            (*c2) = 0;
        (*c1) -= rsize;
        return pattern[(*c2)];
    }
}

u8 cSound::writeToSound(u16 address, u8 value)
{
    switch (address)
    {
        case 0xFF10: //NR 10
            mChannel1->writeRegister(address, value);
            return mChannel1->readRegister(address);
        case 0xFF11: //NR 11
            mChannel1->writeRegister(address, value);
            return mChannel1->readRegister(address);
        case 0xFF12: //NR 12
            mChannel1->writeRegister(address, value);
            return mChannel1->readRegister(address);
        case 0xFF13: //NR 13
            mChannel1->writeRegister(address, value);
            return mChannel1->readRegister(address);
        case 0xFF14: //NR 14
            mChannel1->writeRegister(address, value);
            return mChannel1->readRegister(address);
        case 0xFF16: //NR 21
            ch[1].wavePattern = value >> 6;
            ch[1].keptLength = ch[1].soundLength = (64 - (value & 63)) << 14;
            break;

        case 0xFF17: //NR 22
            ch[1].envInitVol = value >> 4;
            ch[1].envUpdown = (value >> 3) & 1;
            ch[1].envSweep = (value & 7) << 16;
            ch[1].envCounter = ch[1].envSweep;
            return value;
            break;

        case 0xFF18: //NR 23
            ch[1].freq = (ch[1].freq & 0x700) | value;
            return 0xFF;
            break;

        case 0xFF19: //NR 24
            ch[1].freq = (ch[1].freq & 0xFF) | ((value & 7) << 8);
            ch[1].CountCons = (value >> 6) & 1;
            if (value >> 7 == 1) // Trigger
            {
                ch[1].onOff = true;
                mem->IOMap[0xFF26][0] |= 2;
                ch[1].soundLength = ch[1].soundLength == 0 ? 64 << 14 : ch[1].soundLength;
                ch[1].envInitVol = mem->IOMap[0xFF17][0] >> 4;
                ch[1].envSweep = ch[1].envCounter;
            }
            return value | 0xBF;
            break;

        case 0xFF1A: //NR 30
            mChannel3->writeNR30(value);
            return mChannel3->readRegister(address);
            break;

        case 0xFF1B: //NR 31
            mChannel3->writeNR31(value);
            return mChannel3->readRegister(address);

        case 0xFF1C: //NR 32
            mChannel3->writeNR32(value);
            return mChannel3->readRegister(address);

        case 0xFF1D: //NR 33
            mChannel3->writeNR33(value);
            return mChannel3->readRegister(address);

        case 0xFF1E: //NR 34
            mChannel3->writeNR34(value);
            return mChannel3->readRegister(address);

        case 0xFF20: //NR 41
            ch[3].keptLength = ch[3].soundLength = (64 - (value & 63)) << 14;
            return 0xFF;

        case 0xFF21: //NR 42
            ch[3].envInitVol = value >> 4;
            ch[3].envUpdown = (value >> 3) & 1;
            ch[3].envSweep = (value & 7) << 16;
            ch[3].envCounter = ch[3].envSweep;
            break;

        case 0xFF22: //NR 43
            ch[3].sweepShift = 2 << (value >> 4);
            ch[3].sDecInc = (value >> 3) & 1;
            ch[3].sweepCounter = divRatioFreq[(value & 7)];
            break;

        case 0xFF23: //NR 44
            ch[3].CountCons = (value >> 6) & 1;
            if (value >> 7 == 1) // Trigger
            {
                ch[3].onOff = true;
                mem->IOMap[0xFF26][0] |= 8;
                ch[3].soundLength = ch[3].soundLength == 0 ? 64 << 14 : ch[3].keptLength;
                ch[3].envInitVol = mem->IOMap[0xFF21][0] >> 4;
                ch[3].envSweep = ch[3].envCounter;
            }
            return value | 0xBF;

        case 0xFF24: //NR 50
            s1Volumen = value & 7;
            s1 = (value >> 3) & 1;
            s2Volumen = (value >> 4) & 7;
            s2 = (value >> 7) & 1;
            break;
        case 0xFF25:
            //ch[0].s1 = value & 1;
            ch[1].s1 = (value >> 1) & 1;
            //ch[2].s1 = (value >> 2) & 1;
            ch[3].s1 = (value >> 3) & 1;
            //ch[0].s2 = (value >> 4) & 1;
            ch[1].s2 = (value >> 5) & 1;
            //ch[2].s2 = (value >> 6) & 1;
            ch[3].s2 = (value >> 7) & 1;
            break;

        case 0xFF26: //NR 52
            soundActive = (value >> 7) & 1;
            return ((mem->IOMap[0xFF26][0] | mChannel1->getOnOffBit() | mChannel3->getOnOffBit()) & 0x7F) |
                   (soundActive << 7) | 0x70;
        default:
            if (address >= 0xFF30 && address < 0xFF40)
            {
                mChannel3->writeWaveRam(address - 0xFF30, value);
            }
            return 0xFF;
    }
    return value;
}

void cSound::fillBuffer(void)
{
    int sample1, sample2, sample3, sample4;
    int finalSample;

    ch[1].finalFreq = 131072 / (2048 - ch[1].freq);
    ch[3].finalFreq = ch[3].sweepCounter / ch[3].sweepShift;

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
            if (ch[1].onOff)
                sample2 = (ch[1].envInitVol *
                           getCurrentSample(ch[1].finalFreq, ch[1].counter1, wavePatternDuty[ch[1].wavePattern], 8,
                                            ch[1].counter2) / 15);
            if (mChannel3->isOn())
                sample3 = mChannel3->getSample();
            if (ch[3].onOff)
                sample4 = (ch[3].envInitVol *
                           getCurrentNoise(ch[3].finalFreq, &ch[3].counter1, noise, ch[3].sDecInc ? 128 : 32768,
                                           &ch[3].counter2) / 15);
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

    if (ch[1].CountCons)
    {
        ch[1].soundLength -= cycles;
        if (ch[1].soundLength <= 0)
        {
            ch[1].onOff = false;
            mem->IOMap[0xFF26][0] &= 0xFD;
            ch[1].soundLength = 0;
        }
    }
    if (ch[1].envSweep != 0 && ch[1].onOff)
    {
        ch[1].envSweep -= cycles;
        if (ch[1].envSweep <= 0)
        {
            if (ch[1].envUpdown)
            {
                ch[1].envInitVol++;
                if (ch[1].envInitVol > 0xF)
                    ch[1].envInitVol = 0xF;
            }
            else
            {
                ch[1].envInitVol--;
                if (ch[1].envInitVol < 0)
                    ch[1].envInitVol = 0;
            }
            ch[1].envSweep = ch[1].envCounter;
        }
    }
    mChannel3->update(cycles);

    if (ch[3].CountCons)
    {
        ch[3].soundLength -= cycles;
        if (ch[3].soundLength <= 0)
        {
            ch[3].onOff = false;
            mem->IOMap[0xFF26][0] &= 0xF7;
            ch[3].soundLength = 0;
        }
    }
    if (ch[3].envSweep != 0 && ch[3].onOff)
    {
        ch[3].envSweep -= cycles;
        if (ch[3].envSweep <= 0)
        {
            if (ch[3].envUpdown)
            {
                ch[3].envInitVol++;
                if (ch[3].envInitVol > 0xF)
                    ch[3].envInitVol = 0xF;
            }
            else
            {
                ch[3].envInitVol--;
                if (ch[3].envInitVol < 0)
                    ch[3].envInitVol = 0;
            }
            ch[3].envSweep = ch[3].envCounter;
        }
    }
}


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
    if (buffer != NULL) {
        delete buffer;
        buffer = NULL;
    }
}

bool cSound::init(u32 freq, u32 size, u32 bSize)
{
    int i;

    generalFreq = freq;
    soundActive = true;
    ch[0].counter1 = 0;
    ch[0].counter2 = 0;
    ch[0].onOff = false;
    ch[0].wavePattern = 2;
    ch[1].counter1 = 0;
    ch[1].counter2 = 0;
    ch[1].onOff = false;
    ch[1].wavePattern = 2;
    ch[2].counter1 = 0;
    ch[2].counter2 = 0;
    ch[2].onOff = false;
    ch[3].counter1 = 0;
    ch[3].counter2 = 0;
    ch[3].onOff = false;

    memset(wavePatternRam, 0, sizeof (wavePatternRam));
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

    for (i = 0; i < 0x8000; ++i)
        noise[i] = (rand()*100) % 256;

    return true;
}

unsigned char cSound::getCurrentSample(float freq, float *c1, u8 *pattern, s32 size, s32 *c2)
{
    float rsize = generalFreq / freq / size;

    if ((*c1) <= rsize) {
        ++(*c1);
        return pattern[*c2];
    }
    else {
        ++(*c2);
        if ((*c2) >= size)
            (*c2) = 0;
        (*c1) -= rsize - 1;
        return pattern[(*c2)];
    }
}

unsigned char cSound::getCurrentNoise(float freq, float *c1, u8 *pattern, s32 size, s32 *c2)
{
    float rsize = 4194304 / freq / size;

    if ((*c1) <= rsize) {
        ++(*c1);
        return pattern[*c2];
    }
    else {
        ++(*c2);
        if ((*c2) >= size)
            (*c2) = 0;
        (*c1) -= rsize;
        return pattern[(*c2)];
    }
}

void cSound::getSoundMessage(unsigned short address, unsigned char value)
{
    switch (address) {
    case 0xFF10:
        ch[0].sweepTime = ((value >> 4)&7) << 15;
        ch[0].sweepCounter = ch[0].sweepTime;
        ch[0].sDecInc = (value >> 3)&1;
        ch[0].sweepShift = value & 7;
        break;

    case 0xFF11:
        ch[0].wavePattern = value >> 6;
        ch[0].keptLength = ch[0].soundLength = (64 - (value & 63)) << 14;
        break;

    case 0xFF12:
        ch[0].envInitVol = value >> 4;
        ch[0].envUpdown = (value >> 3)&1;
        ch[0].envSweep = (value & 7) << 16;
        ch[0].envCounter = ch[0].envSweep;
        break;

    case 0xFF13:
        ch[0].freq = (ch[0].freq & 0x700) | value;
        break;

    case 0xFF14:
        ch[0].onOff = (value >> 7) == 1 ? true : ch[0].onOff;
        mem->mem[0xFF26][0] = (mem->mem[0xFF26][0]&0xFE) | ((int) ch[0].onOff);
        ch[0].freq = (ch[0].freq & 0xFF) | ((value & 7) << 8);
        ch[0].CountCons = (value >> 6)&1;
        break;

    case 0xFF16:
        ch[1].wavePattern = value >> 6;
        ch[1].keptLength = ch[1].soundLength = (64 - (value & 63)) << 14;
        break;

    case 0xFF17:
        ch[1].envInitVol = value >> 4;
        ch[1].envUpdown = (value >> 3)&1;
        ch[1].envSweep = (value & 7) << 16;
        ch[1].envCounter = ch[1].envSweep;
        break;

    case 0xFF18:
        ch[1].freq = (ch[1].freq & 0x700) | value;
        break;

    case 0xFF19:
        ch[1].onOff = ((value >> 7)&1) == 1 ? true : ch[1].onOff;
        mem->mem[0xFF26][0] = (mem->mem[0xFF26][0]&0xFD) | ((int) ch[1].onOff << 1);
        ch[1].freq = (ch[1].freq & 0xFF) | ((value & 7) << 8);
        ch[1].CountCons = (value >> 6)&1;
        break;

    case 0xFF1A:
        ch[2].onOff = (value >> 7) == 1;
        mem->mem[0xFF26][0] = (mem->mem[0xFF26][0]&0xFB) | ((int) ch[2].onOff << 2);
        break;

    case 0xFF1B:
        ch[2].keptLength = ch[2].soundLength = (256 - value) << 21; //14 OR 21????
        break;

    case 0xFF1C:
        ch[2].envInitVol = (value >> 5)&3;
        break;

    case 0xFF1D:
        ch[2].freq = (ch[2].freq & 0xFF00) | value;
        break;

    case 0xFF1E:
        ch[2].freq = (ch[2].freq & 0xFF) | ((value & 7) << 8);
        ch[2].CountCons = (value >> 6)&1;
        ch[2].onOff = (value >> 7) == 1 ? true : ch[2].onOff;
        mem->mem[0xFF26][0] = (mem->mem[0xFF26][0]&0xFB) | ((int) ch[2].onOff << 2);
        mem->mem[0xFF1A][0] = (mem->mem[0xFF1A][0]&0x7F) | ((int) ch[2].onOff << 7);
        break;

    case 0xFF20:
        ch[3].keptLength = ch[3].soundLength = (64 - (value & 63)) << 14;
        break;

    case 0xFF21:
        ch[3].envInitVol = value >> 4;
        ch[3].envUpdown = (value >> 3)&1;
        ch[3].envSweep = (value & 7) << 16;
        ch[3].envCounter = ch[3].envSweep;
        break;

    case 0xFF22:
        ch[3].sweepShift = 2 << (value >> 4);
        ch[3].sDecInc = (value >> 3)&1;
        ch[3].sweepCounter = divRatioFreq[(value & 7)];
        break;

    case 0xFF23:
        ch[3].onOff = (value >> 7) == 1 ? true : ch[3].onOff;
        mem->mem[0xFF26][0] = (mem->mem[0xFF26][0]&0xF7) | ((int) ch[3].onOff << 3);
        ch[3].CountCons = (value >> 6)&1;
        break;

    case 0xFF24:
        s1Volumen = value & 7;
        s1 = (value >> 3)&1;
        s2Volumen = (value >> 4)&7;
        s2 = (value >> 7)&1;
        break;

    case 0xFF25:
        ch[0].s1 = value & 1;
        ch[1].s1 = (value >> 1)&1;
        ch[2].s1 = (value >> 2)&1;
        ch[3].s1 = (value >> 3)&1;
        ch[0].s2 = (value >> 4)&1;
        ch[1].s2 = (value >> 5)&1;
        ch[2].s2 = (value >> 6)&1;
        ch[3].s2 = (value >> 7)&1;
        break;

    case 0xFF26:
        soundActive = (value >> 7)&1;
        mem->mem[0xFF26][0] = ((u8) soundActive << 7) | ((u8) ch[0].onOff << 3) | ((u8) ch[1].onOff << 2) | ((u8) ch[2].onOff) | ((u8) ch[3].onOff);
        break;
    }
}

void cSound::fillBuffer(void)
{
    u32 i;
    u8 h, l;
    s8 sample1, sample2, sample3, sample4;
    s16 finalSample;


    ch[0].finalFreq = 131072 / (2048 - ch[0].freq);
    ch[1].finalFreq = 131072 / (2048 - ch[1].freq);
    ch[2].finalFreq = 65536 / (2048 - ch[2].freq);
    ch[3].finalFreq = ch[3].sweepCounter / ch[3].sweepShift;

    if (ch[2].onOff) {
        for (i = 0; i < 0x10; ++i) {
            h = mem->mem[0xFF30 + i][0] >> 4;
            l = mem->mem[0xFF30 + i][0]&0xF;
            wavePatternRam[i << 1] = h << 4;
            wavePatternRam[(i << 1) | 1] = l << 4;
        }
    }

    for (i = 0; i < bufferSize; ++i) {
        sample1 = 0;
        sample2 = 0;
        sample3 = 0;
        sample4 = 0;
        if (soundActive) {
            if (ch[0].onOff)
                sample1 = (ch[0].envInitVol * getCurrentSample(ch[0].finalFreq, &ch[0].counter1, wavePatternDuty[ch[0].wavePattern], 8, &ch[0].counter2) / 15);
            if (ch[1].onOff)
                sample2 = (ch[1].envInitVol * getCurrentSample(ch[1].finalFreq, &ch[1].counter1, wavePatternDuty[ch[1].wavePattern], 8, &ch[1].counter2) / 15);
            if (ch[2].onOff)
                sample3 = getCurrentSample(ch[2].finalFreq, &ch[2].counter1, wavePatternRam, 0x20, &ch[2].counter2) >> (ch[2].envInitVol == 0 ? 8 : (ch[2].envInitVol - 1));
            if (ch[3].onOff)
                sample4 = (ch[3].envInitVol * getCurrentNoise(ch[3].finalFreq, &ch[3].counter1, noise, ch[3].sDecInc ? 128 : 32768, &ch[3].counter2) / 15);
        }
        sample1 -= 128;
        sample2 -= 128;
        sample3 -= 128;
        sample4 -= 128;

        finalSample = (sample1 + sample2 + sample3 + sample4) >> 4;

        buffer[i] = (finalSample + 128)&0xFF;

    }
}

void cSound::updateCycles(s32 cycles)
{
    if (ch[0].CountCons && ch[0].onOff) {
        ch[0].soundLength -= cycles;
        if (ch[0].soundLength <= 0) {
            ch[0].soundLength = ch[0].keptLength;
            ch[0].onOff = false;
            mem->mem[0xFF26][0] &= 0xFE;
        }
    }
    if (ch[0].envSweep != 0 && ch[0].onOff) {
        ch[0].envSweep -= cycles;
        if (ch[0].envSweep <= 0) {
            if (ch[0].envUpdown) {
                ch[0].envInitVol++;
                if (ch[0].envInitVol > 0xF)
                    ch[0].envInitVol = 0xF;
            }
            else {
                ch[0].envInitVol--;
                if (ch[0].envInitVol < 0)
                    ch[0].envInitVol = 0;
            }
            ch[0].envSweep = ch[0].envCounter;
        }
    }
    if (ch[0].sweepTime != 0 && ch[0].onOff) {
        ch[0].sweepTime -= cycles;
        if (ch[0].sweepTime <= 0) {
            if (ch[0].sDecInc) {
                ch[0].freq -= ch[0].freq / (1 << ch[0].sweepShift);
                if (ch[0].freq <= 0)
                    ch[0].freq = 0;
            }
            else {
                ch[0].freq += ch[0].freq / (1 << ch[0].sweepShift);
                if (ch[0].freq >= 2048)
                    ch[0].freq = 2047;
            }
            ch[0].sweepTime = ch[0].sweepCounter;
        }
    }


    if (ch[1].CountCons && ch[1].onOff) {
        ch[1].soundLength -= cycles;
        if (ch[1].soundLength <= 0) {
            ch[1].soundLength = ch[1].keptLength;
            ch[1].onOff = false;
            mem->mem[0xFF26][0] &= 0xFD;
        }
    }
    if (ch[1].envSweep != 0 && ch[1].onOff) {
        ch[1].envSweep -= cycles;
        if (ch[1].envSweep <= 0) {
            if (ch[1].envUpdown) {
                ch[1].envInitVol++;
                if (ch[1].envInitVol > 0xF)
                    ch[1].envInitVol = 0xF;
            }
            else {
                ch[1].envInitVol--;
                if (ch[1].envInitVol < 0)
                    ch[1].envInitVol = 0;
            }
            ch[1].envSweep = ch[1].envCounter;
        }
    }
    if (ch[2].CountCons && ch[2].onOff) {
        ch[2].soundLength -= cycles;
        if (ch[2].soundLength <= 0) {
            ch[2].soundLength = ch[2].keptLength;
            ch[2].onOff = false;
            mem->mem[0xFF26][0] &= 0xFB;
            mem->mem[0xFF1A][0] &= 0x7F;
        }
    }

    if (ch[3].CountCons && ch[3].onOff) {
        ch[3].soundLength -= cycles;
        if (ch[3].soundLength <= 0) {
            //ch[3].soundLength=ch[3].keptLength;
            ch[3].onOff = false;
            mem->mem[0xFF26][0] &= 0xF7;
        }
    }
    if (ch[3].envSweep != 0 && ch[3].onOff) {
        ch[3].envSweep -= cycles;
        if (ch[3].envSweep <= 0) {
            if (ch[3].envUpdown) {
                ch[3].envInitVol++;
                if (ch[3].envInitVol > 0xF)
                    ch[3].envInitVol = 0xF;
            }
            else {
                ch[3].envInitVol--;
                if (ch[3].envInitVol < 0)
                    ch[3].envInitVol = 0;
            }
            ch[3].envSweep = ch[3].envCounter;
        }
    }
}


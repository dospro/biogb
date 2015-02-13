/*
 *     Proyect: BioGB
 *    Filename: cSound.h
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rub� Daniel Guti�rez Cruz <dospro@gmail.com>
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


#ifndef BIOGB_SOUND
#define BIOGB_SOUND


#include<stdio.h>
#include<stdlib.h>
#include"macros.h"

class cMemory;

struct soundChannel {
    bool onOff;
    bool s1, s2;

    s32 sweepTime;
    s32 sweepCounter;
    bool sDecInc;
    s32 sweepShift;
    s32 wavePattern;
    s32 soundLength, keptLength;

    s32 envInitVol;
    s32 envCounter;
    bool envUpdown;
    s32 envSweep;

    s32 freq;
    u16 realFreq;
    double finalFreq;
    bool CountCons;

    float counter1;
    s32 counter2;
};

class cSound {
protected:
    soundChannel ch[4];
    bool soundActive;

    bool s1, s2;
    u32 s1Volumen, s2Volumen;

    u8 *buffer;
    u8 wavePatternDuty[4][8];
    u8 wavePatternRam[0x20];
    u8 noise[32768];

    cMemory *mem; //mem Pointer

    u32 generalFreq, bufferSize;

public:
    cSound();
    virtual ~cSound();
    virtual bool init(u32 freq, u32 size, u32 bSize);
    virtual void turnOn(void) = 0;
    virtual void turnOff(void) = 0;

    void getMemoryPointer(cMemory *memp) {
        mem = memp;
    };
    u8 getSoundMessage(u16 address, u8 value);
    u8 getCurrentSample(float freq, float &c1, u8 *pattern, s32 size, s32 &c2);
    u8 getCurrentNoise(float freq, float *c1, u8 *pattern, s32 size, s32 *c2);
    void fillBuffer(void);
    void updateCycles(s32 cycles);
};

#endif

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
#include "cSoundChannel.h"
#include "cSoundChannel1.h"
#include "cSoundChannel3.h"
#include "cSoundChannel2.h"
#include "cSoundChannel4.h"


class cSound
{
public:
    cSound();
    virtual ~cSound();
    virtual bool init(u32 a_frequency, u32 size, u32 bSize);
    virtual void turnOn(void) = 0;
    virtual void turnOff(void) = 0;
    u8 readFromSound(u16 a_address);
    void writeToSound(u16 address, u8 value);
    void fillBuffer(void);
    void updateCycles(s32 cycles);
protected:
    cSoundChannel1 *mChannel1;
    cSoundChannel2 *mChannel2;
    cSoundChannel3 *mChannel3;
    cSoundChannel4 *mChannel4;
    bool soundActive;
    u8 *buffer;
    u32 bufferSize;
    int NR50;
    int NR51;


};

#endif

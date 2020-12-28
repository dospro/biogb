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

#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>

#include "../macros.h"
#include "sound_channel.h"
#include "sound_channel_1.h"
#include "sound_channel_2.h"
#include "sound_channel_3.h"
#include "sound_channel_4.h"

class cSound
{
public:
    cSound(int a_generalFrequency);
    virtual ~cSound();
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    int readFromSound(int a_address);
    void writeToSound(u16 address, u8 value);
    void fillBuffer(u8 *a_internalBuffer, int a_bufferSize);
    void updateCycles(s32 cycles);
protected:
    std::unique_ptr<cSoundChannel1> mChannel1;
    std::unique_ptr<cSoundChannel2> mChannel2;
    std::unique_ptr<cSoundChannel3> mChannel3;
    std::unique_ptr<cSoundChannel4> mChannel4;
    const int GENERAL_FREQUENCY;
    const int NUMBER_OF_CHANNELS;
    bool soundActive;
    int NR50;
    int NR51;


};

#endif

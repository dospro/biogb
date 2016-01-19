/*
 *     Proyect: BioGB
 *    Filename: cPortAudio.h
 *     Version: v4.0
 * Description: Game Boy Color Emulator
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


#ifndef BIOGB_PORTAUDIO
#define BIOGB_PORTAUDIO

#include<SDL.h>
#include "../../sound/cSound.h"


void outputAudio(void *data, u8 *a_internalBuffer, s32 a_bufferSize);

class cSDLSound : public cSound
{
public:
    cSDLSound(int a_generalFrequency, int a_bufferSize);
    virtual ~cSDLSound();
    virtual void turnOn();
    virtual void turnOff();
private:
    SDL_AudioDeviceID mDevice;
    friend void outputAudio(void *data, u8 *a_internalBuffer, s32 a_bufferSize);
};

#endif

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


#include <iostream>
#include"cSDLSound.h"

void outputAudio(void *data, u8 *a_internalBuffer, s32 a_bufferSize)
{
    reinterpret_cast<cSDLSound *>(data)->fillBuffer(a_internalBuffer, a_bufferSize);
}

cSDLSound::cSDLSound(int a_generalFrequency, int a_bufferSize) : cSound(a_generalFrequency)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        throw std::runtime_error("Can't init audio system: " + std::string(SDL_GetError()) + "\n");
    SDL_AudioSpec request;
    SDL_memset(&request, 0, sizeof(request));

    request.freq = a_generalFrequency;
    request.format = AUDIO_U8;
    request.channels = static_cast<Uint8>(NUMBER_OF_CHANNELS);
    request.samples = static_cast<Uint16>(a_bufferSize);
    request.callback = outputAudio;
    request.userdata = this;

    SDL_AudioSpec result;
    mDevice = SDL_OpenAudioDevice(nullptr, 0, &request, &result, 0);
    if (mDevice == 0)
        throw std::runtime_error("Can't open audio device: " + std::string(SDL_GetError()) + "\n");
}

cSDLSound::~cSDLSound()
{
    SDL_CloseAudioDevice(mDevice);
}

void cSDLSound::turnOn()
{
    SDL_PauseAudioDevice(mDevice, 0);
}

void cSDLSound::turnOff()
{
    SDL_PauseAudioDevice(mDevice, 1);
}


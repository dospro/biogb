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


#include"cSDLSound.h"

void outputAudio(void *data, u8 *stream, s32 len)
{
	cSDLSound *tmp=(cSDLSound *)data;
	tmp->fillBuffer();
	memcpy(stream, tmp->buffer, len);
}

cSDLSound::cSDLSound()
{
	desire=NULL;
	device=NULL;
}

cSDLSound::~cSDLSound()
{
	if(desire!=NULL)
	{
		delete desire;
		desire=NULL;
	}
	if(device!=NULL)
	{
		SDL_CloseAudio();
		delete device;
		device=NULL;
	}
}

bool cSDLSound::init(u32 freq, u32 size, u32 bSize)
{
	printf("SDLSound: Iniciando\n");
	desire=new SDL_AudioSpec;
	if(!desire)
		return false;
	device=new SDL_AudioSpec;
	if(!device)
		return false;

	desire->freq=freq;
	desire->format=AUDIO_U8;
	desire->channels=AUDIO_MONO;
	desire->samples=bSize;
	desire->callback=outputAudio;
	desire->userdata=this;
	
	if(SDL_OpenAudio(desire, device)<0)
	{
		delete desire;
		delete device;
		return false;
	}	
	delete desire;
	
    //Call the parent init function
    return cSound::init(freq, size, bSize);

}

void cSDLSound::turnOn(void)
{
	SDL_PauseAudio(0);	
}

void cSDLSound::turnOff(void)
{
	SDL_PauseAudio(1);
}


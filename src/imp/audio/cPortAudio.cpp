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


#include"cPortAudio.h"

cPortAudio::cPortAudio()
{
	stream=NULL;
}

cPortAudio::~cPortAudio()
{
	if(Pa_IsStreamStopped(stream)!=1)
		Pa_CloseStream(stream);
	Pa_Terminate();
	stream=NULL;
}

bool cPortAudio::init(u32 freq, u32 size, u32 bSize)
{
	int i;
	PaError err;
	
	err=Pa_Initialize();
	if(err!=paNoError)
	{
		printf("PA: Error inicializando Port Audio\n");
		return false;
	}

    err=Pa_OpenDefaultStream(&stream, 0, 1, paUInt8, freq, bSize, outputAudio, this);
    if(err!=paNoError)
    {
    	printf("PA: Error creando Stream de audio\n");
    	return false;
    }
    //Call the parent init function
    return cSound::init(freq, size, bSize);

}

void cPortAudio::turnOn(void)
{
	PaError err;
	err=Pa_StartStream(stream);
	if(err!=paNoError)
	{
		printf("PA: No se pudo comenzar la secuencia de audio\n");
	}
	
}

void cPortAudio::turnOff(void)
{
	if(Pa_IsStreamActive(stream)==1)   	
		Pa_StopStream(stream);

}

int outputAudio(const void *in, void *out, unsigned long len, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *data)
{
	cPortAudio *tmp=(cPortAudio*)data;
	tmp->fillBuffer();
	memcpy(out, tmp->buffer, len);
    return 0;
}


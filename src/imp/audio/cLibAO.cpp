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


#include"cLibAO.h"

cLibAO::cLibAO()
{
	device=NULL;
}

cLibAO::~cLibAO()
{
	if(device!=NULL)
	{
		ao_close(device);
	}
	device=NULL;
	ao_shutdown();
}

bool cLibAO::init(u32 freq, u32 size, u32 bSize)
{

	ao_sample_format format;
	int driver_id;
	
	format.bits=bSize;
	format.rate=freq;
	format.channels=1;
	format.byte_format=AO_FMT_LITTLE;
	format.matrix=NULL;
	
	ao_initialize();
	driver_id=ao_default_driver_id();
	if(driver_id==-1)
	{
		printf("libao: No se encontro driver de audio\n");
		return false;
	}
	
	device=ao_open_live(driver_id, &format, NULL);
	if(!device)
	{
		printf("libao: No se pudo inicializar libao\n");
		return false;
	}
    //Call the parent init function
    return cSound::init(freq, size, bSize);

}

void cLibAO::turnOn(void)
{
	
}

void cLibAO::turnOff(void)
{

}

void cLibAO::play(void)
{
	fillBuffer();
	ao_play(device, (char *)buffer, sizeof(buffer));
}


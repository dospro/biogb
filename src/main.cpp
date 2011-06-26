/*
 *     Proyect: BioGB
 *    Filename: main.cpp
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


#include<stdio.h>
#include"cCpu.h"


int main(int argc, char *argv[])
{
	cCpu *gb;
	printf("BioGB v4\n");
	if(argc<2)
	{
		printf("Usage: biogb4 game.gb\n");
		return 0;
	}
	gb=new cCpu;
	if(!gb)
		return 0;
	if(gb->initCpu(argv[1]))
	//if(gb->initCpu("/pc/home/dospro/BioGB_v4_src/roms/zeldadx.gb"))
	{
		//gb->loadSram();
		while(gb->isCpuRunning())
			gb->doCycle();
	}
	delete gb;
	return 0;
}

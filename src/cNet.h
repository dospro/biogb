/*
 *     Proyect: BioGB
 *    Filename: cNet.h
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


#ifndef BIOGB_NET
#define BIOGB_NET

#define SERVER 0xE0
#define CLIENT 0xF0

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL/SDL_net.h>
#include"cCpu.h"

class cNet{
	IPaddress ip, *remoteip;
	TCPsocket server, client;
	s8 message[8];
	s32 len;
	u16 port;

	bool active;
	u8 conType;
public:
	cNet();
	bool init(no);
	void send(u8 data);
	u8 recieve(void);
	void finish(void);

	bool isActive(void);
};

#endif

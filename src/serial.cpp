/*
 *     Proyect: BioGB
 *    Filename: cNet.cpp
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

#ifdef USE_SDL_NET
#include"cNet.h"

cNet::cNet()
{
	active=false;
	conType=0;
}
bool cNet::init(no)
{
	s8 buffer1[256];
	s8 onOff[8], type[8], ipAdr[32];
	FILE *conf;
	conf=fopen("BioGB.net", "r");
	if(!conf)
	{
		ERROR(true, "No se pudo cargar el archivo de red BioGB.net");
		return false;
	}
	do
	{
		fgets(buffer1, 255, conf);
	}while(buffer1[0]=='#' && !feof(conf));
	sscanf(buffer1, "%s %s %d %s", onOff, type, &port, ipAdr);
	fclose(conf);
	if(strcmp(onOff, "off")==0)
	{
		printf("Net is Off\n\n");
		active=false;
		return true;
	}
	printf("Net is On\n\n");
	active=true;

	if(SDLNet_Init()==1)
	{
		ERROR(true,"No se pudo iniciar SDL_Net");
		active=false;
		return false;
	}
	if(strcmp(type, "server")==0)
	{
		//Server code
		//printf("Waiting client at ip:%s port:%s\n", ipAdr, port);
		conType=SERVER;
		SDLNet_ResolveHost(&ip,NULL,port);
		server=SDLNet_TCP_Open(&ip);
		do{
			client=SDLNet_TCP_Accept(server);
		}while(!client);
		remoteip=SDLNet_TCP_GetPeerAddress(client);
		printf("Connected!\n");
	}
	else
	{
		//Client code
		printf("Searching server with ip:%s. Please wait\n", ipAdr);
		conType=CLIENT;
		SDLNet_ResolveHost(&ip,ipAdr,port);
		client=SDLNet_TCP_Open(&ip);
		printf("Connected!\n");
	}
	return true;
}

void cNet::send(u8 data)
{
	SDLNet_TCP_Send(client, &data, 1);
}

u8 cNet::recieve(void)
{
	u8 data;
	u32 len;
	len=SDLNet_TCP_Recv(client, &data, 1);
	if(!len)
		return 0xFF;
	return data;
}

void cNet::finish(void)
{
	if(active)
	{
		active=false;
		if(conType==SERVER)
			SDLNet_TCP_Close(server);
		SDLNet_TCP_Close(client);
		SDLNet_Quit();
	}
}

bool cNet::isActive(void)
{
	return active;
}
#endif


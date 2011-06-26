/*
 *     Proyect: BioGB
 *    Filename: tables.h
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


#ifndef BIOGB_TABLES
#define BIOGB_TABLES

struct romType {
	int id;
	char name[128];
};

struct memSize {
	int id;
	int size;
	int banks;
};
	

struct romType nameList[]={
	{0, "Rom Only"},
	{1, "Rom + MBC1"},
	{2, "Rom + MBC1 + Ram"},
	{3, "Rom + MBC1 + Ram + Battery"},
	{5, "Rom + MBC2"},
	{6, "Rom + MBC2 + Battery"},
	{8, "Rom + Ram"},
	{9, "Rom + Ram + Battery"},
	{0xB, "Rom + MMMO1(Not working)"},
	{0xC, "Rom + MMMO1 + SRam(Not working)"},
	{0xD, "Rom + MMMO1 + SRam + Battery(Not working)"},
	{0xF, "Rom + MBC3 + Timer + Battery(Timer not working)"},
	{0x10, "Rom + MBC3 + Timer + Ram + Battery(Timer not working)"},
	{0x11, "Rom + MBC3"},
	{0x12, "Rom + MBC3 + Ram"},
	{0x13, "Rom + MBC3 + Ram + Battery"},
	{0x15, "Rom + MBC4"},
	{0x16, "Rom + MBC4 + Ram"},
	{0x17, "Rom + MBC4 + Ram + Battery"},
	{0x19, "Rom + MBC5"},
	{0x1A, "Rom + MBC5 + Ram"},
	{0x1B, "Rom + MBC5 + Ram + Battery"},
	{0x1C, "Rom + MBC5 + Rumble"},
	{0x1D, "Rom + MBC5 + Rumble + SRam"},
	{0x1E, "Rom + MBC5 + Rumble + SRam + Battery"},
	{0xFC, "Pocket Cammera(Not working)"},
	{0xFD, "Bandai TAMA5(Not working)"},
	{0xFE, "Hudson HuC-3(Not working)"},
	{0xFF, "Hudson HuC-1 + Ram + Battery"},
};

struct memSize romSizeList[]={
	{0, 16384<<1,	  2},
	{1, 16384<<2,	  4},
	{2, 16384<<3,	  8},
	{3, 16384<<4,	 16},
	{4, 16384<<5,	 32},
	{5, 16384<<6,	 64},
	{6, 16384<<7,	128},
	{7, 16384<<8,	256},
	{8, 16384<<9,	512},
	{0x52, 16384*72, 72},
	{0x53, 16384*80, 80},
	{0x54, 16384*96, 96},
};

struct memSize ramSizeList[]={
	{0, 0,		 0},
	{1, 2048,	 1},
	{2, 8192,	 1},
	{3, 32768,	 4},
	{4, 131072, 16},
};

const unsigned char zero_table[]={
	F_Z,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#endif

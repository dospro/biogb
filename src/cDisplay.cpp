/*
 *     Proyect: BioGB
 *    Filename: cGfx.cpp
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


#include"cDisplay.h"
#include"cCpu.h"

#define COLOR_REDUCTION_FACTOR 1.5
#define COLOR_INFLUENCE_FACTOR 8
#define COLOR_INCREMENT_FACTOR 20

u32 BWColors[2][2];
u32 BGPTable[2][2];
u32 WPTable[2][2];
u32 OBP0Table[2][2];
u32 OBP1Table[2][2];

extern bool isColor;

u32 BGColors[64];
u32 OBJColors[64];

cDisplay::cDisplay()
{
	mem=NULL;
}
cDisplay::~cDisplay()
{
}
bool cDisplay::init(void)
{
	u8 R, G, B;
	u32 i;
	BGPTable[0][0]=0xFFFFFF;
	BGPTable[0][1]=0xC0C0C0;
	BGPTable[1][0]=0x808080;
	BGPTable[1][1]=0x000000;

	WPTable[0][0]=0xFFFFFF;
	WPTable[0][1]=0xC0C0C0;
	WPTable[1][0]=0x808080;
	WPTable[1][1]=0x000000;

	OBP0Table[0][0]=0xFFFFFF;
	OBP0Table[0][1]=0xC0C0C0;
	OBP0Table[1][0]=0x808080;
	OBP0Table[1][1]=0x000000;

	OBP1Table[0][0]=0xFFFFFF;
	OBP1Table[0][1]=0xC0C0C0;
	OBP1Table[1][0]=0x808080;
	OBP1Table[1][1]=0x000000;

	BWColors[0][0]=0xFFFFFF;
	BWColors[0][1]=0xC0C0C0;
	BWColors[1][0]=0x808080;
	BWColors[1][1]=0x000000;
	memset(dirs, 0, sizeof(dirs));
	lcdcPrt=false;

	/*for(i=0; i<0x10000; ++i)
		gbcColors[i]=((((i&0x1F)<<8)>>5)<<16) | (((((i>>5)&0x1F)<<8)>>5)<<8) | ((((i>>10)&0x1F)<<8)>>5);*/
		
	for(i=0; i<0x10000; ++i)
	{
		R=((i&0x1F)<<8)>>5;
		G=(((i>>5)&0x1F)<<8)>>5;
		B=(((i>>10)&0x1F)<<8)>>5;
		
		R=((R/COLOR_REDUCTION_FACTOR)+(G/COLOR_INFLUENCE_FACTOR)+(B/COLOR_INFLUENCE_FACTOR))+COLOR_INCREMENT_FACTOR;
		G=((G/COLOR_REDUCTION_FACTOR)+(R/COLOR_INFLUENCE_FACTOR)+(B/COLOR_INFLUENCE_FACTOR))+COLOR_INCREMENT_FACTOR;
		B=((B/COLOR_REDUCTION_FACTOR)+(R/COLOR_INFLUENCE_FACTOR)+(G/COLOR_INFLUENCE_FACTOR))+COLOR_INCREMENT_FACTOR;
		gbcColors[i]=(R<<16)|(G<<8)|B;
	}

	return true;
}

/*void cDisplay::getDisplayMessage(u16 address, u8 value)
{
	switch(address)
	{
		case 0xFF47://BGP
			BGPTable[1][1]=BWColors[(value>>7)][(value>>6)&1];
			BGPTable[1][0]=BWColors[(value>>5)&1][(value>>4)&1];
			BGPTable[0][1]=BWColors[(value>>3)&1][(value>>2)&1];
			BGPTable[0][0]=BWColors[(value>>1)&1][(value&1)];
			break;
		case 0xFF48://OBP0
			OBP0Table[1][1]=BWColors[(value>>7)][(value>>6)&1];
			OBP0Table[1][0]=BWColors[(value>>5)&1][(value>>4)&1];
			OBP0Table[0][1]=BWColors[(value>>3)&1][(value>>2)&1];
			OBP0Table[0][0]=BWColors[(value>>1)&1][(value&1)];
			break;
		case 0xFF49://OBP1
			OBP1Table[1][1]=BWColors[(value>>7)][(value>>6)&1];
			OBP1Table[1][0]=BWColors[(value>>5)&1][(value>>4)&1];
			OBP1Table[0][1]=BWColors[(value>>3)&1][(value>>2)&1];
			OBP1Table[0][0]=BWColors[(value>>1)&1][(value&1)];
			break;
		case 0xFF69:
			BGColors[mem->mem[0xFF68][0]&0x3F]=value;
			if(mem->mem[0xFF68][0]&0x80)//Autoincrement
				mem->mem[0xFF68][0]++;
			break;
		case 0xFF6B:
			OBJColors[mem->mem[0xFF6A][0]&0x3F]=value;
			if(mem->mem[0xFF6A][0]&0x80)
				mem->mem[0xFF6A][0]++;
			break;
		default:
			break;
	}
}*/

void cDisplay::hBlankDraw(void)
{//Draws a single line
	unsigned char val;
	
	val=mem->mem[0xFF40][0];
	lcdc.lcdcActive=(val>>7)&1;
	lcdc.wndMap=(((val>>6)&1)==1)?0x9C00:0x9800;
	lcdc.wndActive=(val>>5)&1;
	lcdc.bgWndData=(((val>>4)&1)==1)?0x8000:0x8800;
	lcdc.bgMap=(((val>>3)&1)==1)?0x9C00:0x9800;
	lcdc.spSize=(((val>>2)&1)==1)?16:8;
	lcdc.spActive=(val>>1)&1;
	lcdc.bgWndActive=val&1;
	ly=mem->mem[0xFF44][0];


	if(lcdc.lcdcActive)//If the lcd is on
	{
		if(isColor)
		{
			lcdcPrt=lcdc.bgWndActive;
			drawBackGround();
			if(lcdc.wndActive)
				drawWindow();
			if(lcdc.spActive)
				drawSprites();
		}
		else
		{
			lcdcPrt=false;
			if(lcdc.bgWndActive)
			{
				drawBackGround();
				if(lcdc.wndActive)
					drawWindow();
			}
			if(lcdc.spActive)
				drawSprites();
		}
	}
}
void cDisplay::drawBackGround(void)
{
	s32 x, i, j;
	u8 y;
	u8 tId; 
	s32 dir;
	u8 tData1, tData2;
	u8 bank=0, pal=0, hFlip=0, vFlip=0;
	s32 tileCounter=mem->mem[0xFF43][0]>>3;

	x=-(mem->mem[0xFF43][0]&7);
	y=(ly+mem->mem[0xFF42][0])&0xFF;

	//Go through all the line tile by tile
	for(i=x; i<160; i+=8)
	{
		if(tileCounter>=32)
			tileCounter-=32;
		//We get the tile Id
		dir=lcdc.bgMap+tileCounter+((y>>3)<<5);
		if(isColor)
		{
			hFlip=(mem->mem[dir][1]>>5)&1;
			vFlip=(mem->mem[dir][1]>>6)&1;
			
			/*dirs[tileCounter][y>>3]=*/bgPrt=(mem->mem[dir][1]>>7)&1;
			
			bank=(mem->mem[dir][1]>>3)&1;
			pal=(mem->mem[dir][1]&7)<<3;
		
			BGPTable[0][0]=gbcColors[((BGColors[pal+1])<<8)|(BGColors[pal])];
			BGPTable[0][1]=gbcColors[((BGColors[pal+3])<<8)|(BGColors[pal+2])];
			BGPTable[1][0]=gbcColors[((BGColors[pal+5])<<8)|(BGColors[pal+4])];
			BGPTable[1][1]=gbcColors[((BGColors[pal+7])<<8)|(BGColors[pal+6])];
		}
		if(lcdc.bgWndData==0x8000)
			tId=mem->mem[dir][0];
		else
			tId=(mem->mem[dir][0]^0x80);
		if(vFlip==1)
		{
			tData1=mem->mem[lcdc.bgWndData+(tId<<4)+((7-(y&7))<<1)+1][bank];
			tData2=mem->mem[lcdc.bgWndData+(tId<<4)+((7-(y&7))<<1)  ][bank];
		}
		else
		{
			tData1=mem->mem[lcdc.bgWndData+(tId<<4)+((y&7)<<1)+1][bank];
			tData2=mem->mem[lcdc.bgWndData+(tId<<4)+((y&7)<<1)  ][bank];
		}
		if(hFlip==1)
		{
			for(j=0; j<8; j++)
			{
				if(i+j<160 && i+j>=0 && ly<144 && ly>=0)
					videoBuffer[i+j][ly]=BGPTable[tData1&1][tData2&1];

				tData1>>=1;
				tData2>>=1;
			}
		}
		else
		{
			for(j=7; j>=0; j--)
			{
				if(i+j<160 && i+j>=0 && ly<144 && ly>=0)
					videoBuffer[i+j][ly]=BGPTable[tData1&1][tData2&1];

				tData1>>=1;
				tData2>>=1;
			}
		}
		++tileCounter;
	}
}

void cDisplay::drawWindow(void)
{
	int tileCounter, tId, dir;
	unsigned char tData1, tData2;
	int bank=0, pal=0;
	int wx, wy, x, y;
	int i, j;

	wx=mem->mem[0xFF4B][0]-7;
	wy=mem->mem[0xFF4A][0];

	x=(wx&7);
	y=(ly-wy)&0xFF;
	tileCounter=-(wx>>3);
	if(ly<144 && ly>=wy && wx<=166 && wy>=0 && wy<=143)
	{
		for(i=x; i<160; i+=8)
		{
			if(tileCounter>=32)
				tileCounter-=32;
			dir=lcdc.wndMap+tileCounter+((y>>3)<<5);
			tId=mem->mem[dir][0];
			if(isColor)
			{
				bank=(mem->mem[dir][1]>>3)&1;
				pal=(mem->mem[dir][1]&7)<<3;
			}
			if(lcdc.bgWndData==0x8800)
				tId^=0x80;
			tData1=mem->mem[lcdc.bgWndData+(tId<<4)+((y&7)<<1)+1][bank];
			tData2=mem->mem[lcdc.bgWndData+(tId<<4)+((y&7)<<1)  ][bank];

			if(isColor)
			{
				WPTable[0][0]=gbcColors[((BGColors[pal+1])<<8)|(BGColors[pal])];
				WPTable[0][1]=gbcColors[((BGColors[pal+3])<<8)|(BGColors[pal+2])];
				WPTable[1][0]=gbcColors[((BGColors[pal+5])<<8)|(BGColors[pal+4])];
				WPTable[1][1]=gbcColors[((BGColors[pal+7])<<8)|(BGColors[pal+6])];
			}

			for(j=7; j>=0; j--)
			{
				if(i+j<160 && i+j>=0 && i+j>=wx)
						videoBuffer[i+j][ly]=WPTable[tData1&1][tData2&1];
				tData1>>=1;
				tData2>>=1;
			}
			++tileCounter;
		}
	}
}

void cDisplay::drawSprites(void)
{
	int y;
	//int tempx, tempy;
	unsigned char tData1, tData2;
	int i, p;
	for(i=0; i<40; ++i)
	{
		sprite.y=mem->mem[0xFE00+(i<<2)][0]-16;
		sprite.x=mem->mem[0xFE00+(i<<2)+1][0]-8;
		sprite.patNum=lcdc.spSize==16?mem->mem[0xFE00+(i<<2)+2][0]&0xFE:mem->mem[0xFE00+(i<<2)+2][0];
		oamPrt=sprite.priority=(mem->mem[0xFE00+(i<<2)+3][0]>>7)&1;
		sprite.yFlip=(mem->mem[0xFE00+(i<<2)+3][0]>>6)&1;
		sprite.xFlip=(mem->mem[0xFE00+(i<<2)+3][0]>>5)&1;
		sprite.palette=(mem->mem[0xFE00+(i<<2)+3][0]>>4)&1;
		sprite.bank=0;
		sprite.cgbPalete=0;
		prtSym=4;	

		if(isColor)
		{
			sprite.bank=(mem->mem[0xFE00+(i<<2)+3][0]>>3)&1;
			sprite.cgbPalete=(mem->mem[0xFE00+(i<<2)+3][0]&7)<<3;
			OBP0Table[0][0]=gbcColors[((OBJColors[sprite.cgbPalete+1])<<8)|(OBJColors[sprite.cgbPalete])];
			OBP0Table[0][1]=gbcColors[((OBJColors[sprite.cgbPalete+3])<<8)|(OBJColors[sprite.cgbPalete+2])];
			OBP0Table[1][0]=gbcColors[((OBJColors[sprite.cgbPalete+5])<<8)|(OBJColors[sprite.cgbPalete+4])];
			OBP0Table[1][1]=gbcColors[((OBJColors[sprite.cgbPalete+7])<<8)|(OBJColors[sprite.cgbPalete+6])];
			sprite.palette=false;
			if(lcdcPrt)
			{
				if(!bgPrt)
				{
					if(oamPrt)
						prtSym=5;//OAM behind colors 123
					else
						prtSym=4;//OAM above bg
				}
				else
					prtSym=3;//BG above OAM
			}
			else
				prtSym=0;//BG above everything
		}

		if(ly>=sprite.y && ly<sprite.y+lcdc.spSize)
		{
			if(sprite.yFlip)
				y=(lcdc.spSize-1)-(ly-sprite.y);
			else
				y=ly-sprite.y;

			tData1=mem->mem[0x8000+(sprite.patNum<<4)+((y&(lcdc.spSize-1))<<1)+1][sprite.bank];
			tData2=mem->mem[0x8000+(sprite.patNum<<4)+((y&(lcdc.spSize-1))<<1)  ][sprite.bank];

			
			if(sprite.xFlip)
			{
				for(p=0; p<8; ++p)
				{
					switch(prtSym)
					{
						case 0:
							break;
						case 3:
							break;								
						case 4:
							if(sprite.x+p<160 && sprite.x+p>=0 && ly>=0 && ly<144 && ((tData1&1)|(tData2&1))!=0)
								videoBuffer[sprite.x+p][ly]=sprite.palette?OBP1Table[tData1&1][tData2&1]:OBP0Table[tData1&1][tData2&1];
							break;
						case 5:
							if(sprite.x+p<160 && sprite.x+p>=0 && ly>=0 && ly<144 && ((tData1&1)|(tData2&1))!=0 && (videoBuffer[sprite.x+p][ly]==BGPTable[0][0] && videoBuffer[sprite.x+p][ly]==WPTable[0][0]))
								videoBuffer[sprite.x+p][ly]=sprite.palette?OBP1Table[tData1&1][tData2&1]:OBP0Table[tData1&1][tData2&1];
							break;
					}
					tData1>>=1;
					tData2>>=1;
				}
			}
			else
			{
				for(p=7; p>=0; --p)
				{
					switch(prtSym)
					{
						case 0:
							break;
						case 3:
							break;
						case 4:
							if(sprite.x+p<160 && sprite.x+p>=0 && ly>=0 && ly<144 && ((tData1&1)+(tData2&1))!=0)
								videoBuffer[sprite.x+p][ly]=sprite.palette?OBP1Table[tData1&1][tData2&1]:OBP0Table[tData1&1][tData2&1];
							break;
						case 5:
							if(sprite.x+p<160 && sprite.x+p>=0 && ly>=0 && ly<144 && ((tData1&1)+(tData2&1))!=0 && (videoBuffer[sprite.x+p][ly]==BGPTable[0][0] && videoBuffer[sprite.x+p][ly]==WPTable[0][0]))
								videoBuffer[sprite.x+p][ly]=sprite.palette?OBP1Table[tData1&1][tData2&1]:OBP0Table[tData1&1][tData2&1];
							break;
					}
					tData1>>=1;
					tData2>>=1;
				}
			}
		}
	}
}

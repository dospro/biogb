/*
 *     Proyect: BioGB
 *    Filename: cCpu.h
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rubén Daniel Gutiérrrez Cruz <dospro@gmail.com>
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

#ifndef BIOGB_CPU
#define BIOGB_CPU

#include<stdio.h>
#include<stdlib.h>
#include"cInput.h"
#include"cDisplay.h"
#include"cSound.h"
#include"macros.h"

#ifdef USE_SDL_NET
#include"cNet.h"
#endif

struct gbHeader{
	char name[16];
	u8 color;
	u8 mbc;
	u8 romSize;
	u8 ramSize;
};

struct HDMA {
	u8 hs, ls;
	u8 hd, ld;
	u16 length;
	bool mode, active;
};

struct RTC_Regs{
	u8 rtcRegSelect;
	bool areRtcRegsSelected;
	bool latch;

	u8 sec;
	u8 min;
	u8 hr;
	u8 dl;
	u8 dh;
};

struct SerialTransfer{
	bool start;
	bool cType;
	bool speed;
	u8 rec, trans;
};

class cMemory{
	HDMA hdma;
	u8 mm;
	bool ramEnable;
	u8 hi, lo;
	u16 dest, source;

	void DMATransfer(u8 address);
	void HDMATransfer(u16 source, u16 dest, u32 length);
public:
	gbHeader info;
	RTC_Regs rtc, rtc2;
	SerialTransfer ST;
	u16 romBank, ramBank, wRamBank, vRamBank;
	u8 mem[0x10000][0x200];
	
	void rtcCounter(void);
	bool loadRom(char *file);
	u8 readByte(u16);
	void writeByte(u16, u8);
	void HBlankHDMA(void);
};

class cCpu {
	//Private Data
	cMemory *mem;
	cInput *input;
	cDisplay *display;

	FILE *log;
	u8 a, b, c, d ,e ,h, l;
	bool zf, nf, hf, cf;
	u16 pc, sp;
	bool interruptsEnabled;
	u32 intStatus;
	u32 timerCounter;
	u8 opCycles[256];

	s32 cyclesCount;
	u8 nextMode;
	u8 scanLine;
	s32 lyCycles;
	
	/*s32 allModes;
	s32 mode0;
	s32 mode1;
	s32 mode2;
	s32 mode3;*/
	s32 divideReg;
	s32 rtcCount;


	bool isRunning;

	u32 fps, fpsCounter;
	u32 time1, time2;
	bool SpeedkeyChange;
	u32 fpsSpeed;

	//Help routines
	inline u8 flags(void);
	inline void flags(u8);

	inline	u16 af(void);
	inline void af(u16);

	inline	u16 bc(void);
	inline void bc(u16);

	inline u16 de(void);
	inline void de(u16);

	inline u16 hl(void);
	inline void hl(u16);

	inline u8  readNextByte(void);
	inline u16 readNextWord(void);

	//Cpu instructions routines
	inline void adc(u8 val);
	inline void add(u8 val);
	inline void addhl(u16 val);
	inline void addsp(s8 val);
	inline void z8and(u8 val);
	inline void bit(u8 bit, u8 reg);
	inline void call(bool condition, u16 address);
	inline void cp(u8 val);
	inline void daa(void);
	inline void dec(u8 *reg);
	inline void dec(u8 *r1, u8 *r2);
	inline void dechl(void);
	inline void inc(u8 *reg);
	inline void inc(u8 *r1, u8 *r2);
	inline void inchl(void);
	inline void jp(bool condition, u16 address);
	inline void jr(bool condition, s8 val);
	inline void ldhl(s8 val);
	inline void ldnnsp(u16 val);
	inline void z8or(u8 val);
	inline void pop(u8 *r1, u8 *r2);
	inline void popaf(void);
	inline void push(u16 regs);
	inline void res(u8 bit, u8 *reg);
	inline void reshl(u8 bit);
	inline void ret(bool condition);
	inline void rlc(u8 *reg);
	inline void rlca(void);
	inline void rlchl(void);
	inline void rl(u8 *reg);
	inline void rla(void);
	inline void rlhl(void);
	inline void rrc(u8 *reg);
	inline void rrca(void);
	inline void rrchl(void);
	inline void rr(u8 *reg);
	inline void rra(void);
	inline void rrhl(void);
	inline void rst(u8 val);
	inline void sbc(u8 val);
	inline void set(u8 bit, u8 *reg);
	inline void sethl(u8 bit);
	inline void sla(u8 *reg);
	inline void slahl(void);
	inline void sra(u8 *reg);
	inline void srahl(void);
	inline void srl(u8 *reg);
	inline void srlhl(void);
	inline void sub(u8 val);
	inline void swap(u8 *reg);
	inline void swaphl(void);
	inline void z8xor(u8 val);

	void setMode(int mode);
	void setInterrupt(int interrupt);
	void executeOpcode(void);
	void checkInterrupts(void);
	void updateModes(void);
	void updateTimer(int cycles);
	void initRTCTimer(void);
	void fullUpdate(void);

public:
	//Interface
	cCpu();
	~cCpu();
	bool initCpu(char *file);
	bool isCpuRunning(void){return isRunning;}
	void doCycle(void);

	void saveSram(void);
	void loadSram(void);
	void saveState(int number);
	void loadState(int number);
};

#endif

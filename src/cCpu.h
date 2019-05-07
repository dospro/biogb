#ifndef BIOGB_CPU
#define BIOGB_CPU

#include<stdio.h>
#include<stdlib.h>
#include "MemoryMap.h"
#include"cInput.h"
#include"sound/cSound.h"
#include"macros.h"

#ifdef USE_SDL_NET
#include"cNet.h"
#endif


class cCpu
{
public:

    cCpu();
    ~cCpu();
    bool init_cpu(std::string file_name);

    bool isCpuRunning()
    { return isRunning; }

    void doCycle();
    void saveState(int number);
    void loadState(int number);
    int fetchOpcode();
    void updateCycles(int a_opcode);

private:

    MemoryMap *mMemory;
    std::array<int, 0x100> mOpcodeCyclesTable;
    std::array<int, 0x100> opCycles;
    std::array<int, 0x100> mCBOpcodeCyclesTable;
    u8 a, b, c, d, e, h, l;
    bool z_flag, n_flag, h_flag, c_flag;
    u16 pc, sp;
    bool interruptsEnabled;
    u32 intStatus;
    s32 cyclesCount;
    int mCyclesSum;
    u8 nextMode;
    u8 scanLine;
    s32 lyCycles;
    s32 rtcCount;
    int mCurrentSpeed;
    bool isRunning;
    u32 fps, fpsCounter;
    u32 time1, time2;
    bool SpeedkeyChange;
    u32 fpsSpeed;
    //Help routines
    inline u8 flags();
    inline void flags(u8);
    inline u16 af();
    inline void af(u16);
    inline u16 bc();
    inline void bc(u16);
    inline u16 de();
    inline void de(u16);
    inline u16 hl();
    inline void hl(u16);
    inline u8 readNextByte();
    inline u16 readNextWord();
    //Cpu instructions routines
    inline void adc(u8 value);
    inline void add(u8 value);
    inline void addhl(u16 value);
    inline void addsp(s8 value);
    inline void z8and(u8 value);
    inline void bit(u8 bit_number, u8 register_value);
    inline void call(bool condition, u16 address);
    inline void cp(u8 val);
    inline void daa();
    inline void dec(u8 &reg);
    inline void dec(u8 &high_register, u8 &low_register);
    inline void dechl();
    inline void inc(u8 &reg);
    inline void inc(u8 &high_register, u8 &low_register);
    inline void inchl();
    inline void jp(bool condition, u16 address);
    inline void jr(bool condition, s8 value);
    inline void ldhl(s8 value);
    inline void ldnnsp(u16 value);
    inline void z8or(u8 value);
    inline void pop(u8 &high_register, u8 &low_register);
    inline void popaf();
    inline void push(u16 regs);
    inline void res(u8 bit_number, u8 &reg);
    inline void reshl(u8 bit_number);
    inline void ret(bool condition);
    inline void rlc(u8 &reg);
    inline void rlca();
    inline void rlchl();
    inline void rl(u8 &reg);
    inline void rla();
    inline void rlhl();
    inline void rrc(u8 &reg);
    inline void rrca();
    inline void rrchl();
    inline void rr(u8 &reg);
    inline void rra();
    inline void rrhl();
    inline void rst(u8 address);
    inline void sbc(u8 value);
    inline void set(u8 bit, u8 &reg);
    inline void sethl(u8 bit_number);
    inline void sla(u8 &reg);
    inline void slahl();
    inline void sra(u8 &reg);
    inline void srahl();
    inline void srl(u8 &reg);
    inline void srlhl();
    inline void sub(u8 value);
    inline void swap(u8 &reg);
    inline void swaphl();
    inline void z8xor(u8 val);
    void setMode(int mode);
    void executeOpcode(int a_opcode);
    void checkInterrupts();
    void updateModes();
    void initRTCTimer();
    void fullUpdate();
};

#endif

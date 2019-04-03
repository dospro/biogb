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
    bool zf, nf, hf, cf;
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
    inline void adc(u8 val);
    inline void add(u8 val);
    inline void addhl(u16 val);
    inline void addsp(s8 val);
    inline void z8and(u8 val);
    inline void bit(u8 bit, u8 reg);
    inline void call(bool condition, u16 address);
    inline void cp(u8 val);
    inline void daa(void);
    inline void dec(u8 &reg);
    inline void dec(u8 &r1, u8 &r2);
    inline void dechl(void);
    inline void inc(u8 &reg);
    inline void inc(u8 &r1, u8 &r2);
    inline void inchl(void);
    inline void jp(bool condition, u16 address);
    inline void jr(bool condition, s8 val);
    inline void ldhl(s8 val);
    inline void ldnnsp(u16 val);
    inline void z8or(u8 val);
    inline void pop(u8 &r1, u8 &r2);
    inline void popaf(void);
    inline void push(u16 regs);
    inline void res(u8 bit, u8 &reg);
    inline void reshl(u8 bit);
    inline void ret(bool condition);
    inline void rlc(u8 &reg);
    inline void rlca(void);
    inline void rlchl(void);
    inline void rl(u8 &reg);
    inline void rla(void);
    inline void rlhl(void);
    inline void rrc(u8 &reg);
    inline void rrca(void);
    inline void rrchl(void);
    inline void rr(u8 &reg);
    inline void rra(void);
    inline void rrhl(void);
    inline void rst(u8 val);
    inline void sbc(u8 val);
    inline void set(u8 bit, u8 &reg);
    inline void sethl(u8 bit);
    inline void sla(u8 &reg);
    inline void slahl(void);
    inline void sra(u8 &reg);
    inline void srahl(void);
    inline void srl(u8 &reg);
    inline void srlhl(void);
    inline void sub(u8 val);
    inline void swap(u8 &reg);
    inline void swaphl(void);
    inline void z8xor(u8 val);
    void setMode(int mode);
    void executeOpcode(int a_opcode);
    void checkInterrupts();
    void updateModes();
    void initRTCTimer();
    void fullUpdate();
};

#endif

#include <iostream>
#include <fstream>
#include <cstring>
#include"cCpu.h"


#ifdef USE_SDL_NET
cNet net;
#endif


bool isColor;

cCpu::cCpu() : mCyclesSum{0}, mCurrentSpeed{0}
{
    mMemory = nullptr;
    mOpcodeCyclesTable = {1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
                          0, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
                          2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
                          2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                          2, 3, 3, 3, 3, 4, 2, 4, 2, 4, 3, 0, 3, 3, 2, 4,
                          2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,
                          3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
                          3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4};
    for (int &i : mOpcodeCyclesTable)
    {
        i <<= 2;
    }


    mCBOpcodeCyclesTable = {2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
                            2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
                            2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
                            2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
                            2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2};
    for (int &i : mCBOpcodeCyclesTable)
    {
        i <<= 2;
    }
}

cCpu::~cCpu()
{
    if (mMemory)
        delete mMemory;
}

u8 cCpu::flags() {
    return (u8)(z_flag << 7u) | (u8)(n_flag << 6u) | (u8)(h_flag << 5u) | (u8)(c_flag << 4u);
}

void cCpu::flags(u8 value) {
    z_flag = (u8)(value >> 7u) & 1u;
    n_flag = (u8)(value >> 6u) & 1u;
    h_flag = (u8)(value >> 5u) & 1u;
    c_flag = (u8)(value >> 4u) & 1u;
}

u16 cCpu::af() {
    return (u16)(a << 8u) | flags();
}

void cCpu::af(u16 value) {
    a = value >> 8u;
    flags(value & 0xFFu);
}

u16 cCpu::bc() {
    return (u16)(b << 8u) | c;
}

void cCpu::bc(u16 value) {
    b = value >> 8u;
    c = value & 0xFFu;
}

u16 cCpu::de() {
    return (u16)(d << 8u) | e;
}

void cCpu::de(u16 value) {
    d = value >> 8u;
    e = value & 0xFFu;
}

u16 cCpu::hl() {
    return (u16)(h << 8u) | l;
}

void cCpu::hl(u16 value) {
    h = value >> 8u;
    l = value & 0xFFu;
}

u8 cCpu::readNextByte() {
    return mMemory->readByte(pc++);
}

u16 cCpu::readNextWord() {
    u16 result = (mMemory->readByte(pc) | (mMemory->readByte(pc + 1) << 8));
    pc += 2;
    return result;
}

void cCpu::saveState(int number)
{

}

void cCpu::loadState(int number)
{

}

bool cCpu::init_cpu(std::string file_name)
{
    cyclesCount = 0;
    nextMode = 3;
    lyCycles = 456;
    scanLine = 0;


    af(0x11B0);
    bc(0x0013);
    de(0x00D8);
    hl(0x014D);
    pc = 0x0100;
    sp = 0xFFFE;


    std::cout << "Rom....";
    mMemory = new MemoryMap;
    if (mMemory == nullptr)
    {
        std::cout << "Failure Type A" << std::endl;
        return false;
    }
    if (!mMemory->load_rom(file_name))
    {
        std::cout << "Failure Type B" << std::endl;
        return false;
    }
    std::cout << "Succeded" << std::endl;
#ifdef USE_SDL_NET
    std::cout << "OPTIONAL Net...";
    if (!net.init()) {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << "OK" << std::endl;
#endif

    initRTCTimer();


    mMemory->writeByte(0xFF05, 0x00);
    mMemory->writeByte(0xFF06, 0x00);
    mMemory->writeByte(0xFF07, 0x00);
    mMemory->writeByte(0xFF10, 0x00);
    mMemory->writeByte(0xFF11, 0xBF);
    mMemory->writeByte(0xFF12, 0xF2);
    mMemory->writeByte(0xFF14, 0xBF);
    mMemory->writeByte(0xFF16, 0x3F);
    mMemory->writeByte(0xFF17, 0x00);
    mMemory->writeByte(0xFF19, 0xBF);
    mMemory->writeByte(0xFF1A, 0x7F);
    mMemory->writeByte(0xFF1B, 0xFF);
    mMemory->writeByte(0xFF1C, 0x9F);
    mMemory->writeByte(0xFF1E, 0xBF);
    mMemory->writeByte(0xFF20, 0xFF);
    mMemory->writeByte(0xFF21, 0x00);
    mMemory->writeByte(0xFF22, 0x00);
    mMemory->writeByte(0xFF23, 0xBF);
    mMemory->writeByte(0xFF24, 0x77);
    mMemory->writeByte(0xFF25, 0xF3);
    mMemory->writeByte(0xFF26, 0xF1);
    mMemory->writeByte(0xFF40, 0x91);
    mMemory->writeByte(0xFF42, 0x00);
    mMemory->writeByte(0xFF43, 0x00);
    mMemory->writeByte(0xFF45, 0x00);
    mMemory->writeByte(0xFF47, 0xFC);
    mMemory->writeByte(0xFF48, 0xFF);
    mMemory->writeByte(0xFF49, 0xFF);
    mMemory->writeByte(0xFF4A, 0x00);
    mMemory->writeByte(0xFF4B, 0x00);
    mMemory->writeByte(0xFF4D, 0x00);
    mMemory->writeByte(0xFFFF, 0x00);


    time1 = SDL_GetTicks() + 16;
    time2 = SDL_GetTicks() + 1000;

    isRunning = true;
    fpsSpeed = 1;
    SpeedkeyChange = false;

    std::cout << "Everything Done!!" << std::endl;
    return true;
}

void cCpu::setMode(int mode)
{
    switch (mode)
    {
        case 0:
            mMemory->IOMap[0xFF41][0] &= 252;
            break;
        case 1:
            mMemory->IOMap[0xFF41][0] = (mMemory->IOMap[0xFF41][0] & 252) | 1;
            break;
        case 2:
            mMemory->IOMap[0xFF41][0] = (mMemory->IOMap[0xFF41][0] & 252) | 2;
            break;
        case 3:
            mMemory->IOMap[0xFF41][0] = (mMemory->IOMap[0xFF41][0] & 252) | 3;
            break;
        case 4:
            mMemory->IOMap[0xFF41][0] |= 4;
            break;
    }
}

void cCpu::checkInterrupts()
{
    int interrupt;
    interrupt = mMemory->mInterrupts->getReadyInterrupts();
    if (interruptsEnabled && interrupt > 0)
    {
        interruptsEnabled = false;
        if (interrupt & 1)//v-blank
        {
            call(true, 0x0040);
            mMemory->mInterrupts->resetInterrupt(cInterrupts::VBLANK);
            mCyclesSum += 20;
        }
        else if (interrupt & 2)//LCDC
        {
            call(true, 0x0048);
            mMemory->mInterrupts->resetInterrupt(cInterrupts::LCDC);
            mCyclesSum += 20;
        }
        else if (interrupt & 4)//timer
        {
            call(true, 0x0050);
            mMemory->mInterrupts->resetInterrupt(cInterrupts::TIMER);
            mCyclesSum += 20;
        }
        else if (interrupt & 8)//Serial Transfer
        {
            call(true, 0x0058);
            mMemory->mInterrupts->resetInterrupt(cInterrupts::SERIAL);
            mCyclesSum += 20;
        }
        else if (interrupt & 16)//P10-P13
        {
            call(true, 0x0060);
            mMemory->mInterrupts->resetInterrupt(cInterrupts::JOYPAD);
            mCyclesSum += 20;
        }
    }
}

void cCpu::initRTCTimer()
{
    struct tm currentTime;
    time_t timer;

    timer = time(NULL);
    currentTime = *localtime(&timer);
    //printf("%d:%d:%d\n", currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec);

    mMemory->rtc.sec = currentTime.tm_sec;
    mMemory->rtc.min = currentTime.tm_min;
    mMemory->rtc.hr = currentTime.tm_hour - 10;
    mMemory->rtc.dl = currentTime.tm_wday;

    mMemory->rtc2.sec = currentTime.tm_sec;
    mMemory->rtc2.min = currentTime.tm_min;
    mMemory->rtc2.hr = currentTime.tm_hour - 10;
    mMemory->rtc2.dl = currentTime.tm_wday;
}

void cCpu::doCycle()
{
    u8 opCode = fetchOpCode();
    updateIMEFlag();
    if(opCode == 0xCB) {
        u8 cbOpCode = fetchOpCode();
        executeCBOpCode(cbOpCode);
        mCyclesSum += mCBOpcodeCyclesTable[cbOpCode];
    } else {
        executeOpCode(opCode);
        mCyclesSum += mOpcodeCyclesTable[opCode];
    }
    updateCycles();
    checkInterrupts();
    updateModes();


    if (rtcCount >= 4194304)
    {
        rtcCount -= 4194304;
        mMemory->rtcCounter();
    }
}

int cCpu::fetchOpCode()
{
    return mMemory->readByte(pc++);
}

void cCpu::updateCycles()
{
    mMemory->updateIO(mCyclesSum);
    cyclesCount -= mCyclesSum;
    lyCycles -= mCyclesSum;
    rtcCount += mCyclesSum;
    mCyclesSum = 0;
}

void cCpu::updateModes()
{
    //NOTE: When CGB is at double speed LCD, Sound and HDMA work as normal.
    //This means those take double clock cycles to finish(because those are
    // slower than the other parts).
    auto LCDC = mMemory->readByte(0xFF40);
    if (lyCycles <= 0) {
        lyCycles += (456 << mCurrentSpeed);
        scanLine = ++mMemory->IOMap[0xFF44][0]; //Increment LY
        if (scanLine == 153) {
            scanLine = 0;
            mMemory->IOMap[0xFF44][0] = 0;
        }

        auto LYC = mMemory->readByte(0xFF45);
        if (scanLine == LYC) {
            // We have a LY==LYC interrupt
            if ((mMemory->IOMap[0xFF41][0] & 0x40) && LCDC & 0x80)
                mMemory->mInterrupts->setInterrupt(cInterrupts::LCDC);
            setMode(4); //Set LYC flag(no mode)
        }
    }

    if (cyclesCount <= 0)//If the current mode has finished, change mode
    {
        switch (nextMode)
        {
            case 0://Do Mode 0 actions.
                cyclesCount += (204 << mCurrentSpeed); //Number of cycles this mode needs
                nextMode = 2;

                setMode(0);
                mMemory->mDisplay->hBlankDraw();
                if ((mMemory->IOMap[0xFF41][0] & 8) && (LCDC & 0x80))
                    mMemory->mInterrupts->setInterrupt(cInterrupts::LCDC); //Mode 0 H-Blank LCDC Interrupt
                if (isColor)//In Gameboy Color it must be checked if we need to do hdma transfers
                    mMemory->HBlankHDMA();

                if (scanLine == 143)//If next scanline is 144 then go to V-Blank period
                    nextMode = 1;
                break;

            case 1://Do Mode 1 actions
                cyclesCount += (4560 << mCurrentSpeed);
                nextMode = 4; //Full update
                //display->updateScreen();
                setMode(1);
                if (LCDC & 0x80)
                    mMemory->mInterrupts->setInterrupt(cInterrupts::VBLANK);
                if (mMemory->IOMap[0xFF41][0] & 0x10 && LCDC & 0x80)
                    mMemory->mInterrupts->setInterrupt(cInterrupts::LCDC); //Mode 1 V-Blank LCDC Interrupt
                break;

            case 2://Do Mode 2 actions
                cyclesCount += (80 << mCurrentSpeed);
                nextMode = 3;
                setMode(2);
                if (mMemory->IOMap[0xFF41][0] & 0x20 && LCDC >> 7 == 1)
                    mMemory->mInterrupts->setInterrupt(cInterrupts::LCDC); //Mode 2 OAM LCDC Interrupt
                break;

            case 3://Do Mode 3 actions
                cyclesCount += (172 << mCurrentSpeed);
                nextMode = 0;
                setMode(3);
                break;
            case 4://Full update after mode 1
                nextMode = 2;
                mMemory->mDisplay->updateScreen();
                fullUpdate();
                break;
        }
    }
}

void cCpu::fullUpdate()
{
    mMemory->mInput->update();
    if (mMemory->mInput->isKeyPressed(GBK_ESCAPE))
    {
        mMemory->save_sram();
        mMemory->mSound->turnOff();
        isRunning = false;
    }
    if (mMemory->mInput->isKeyPressed(GBK_s))
    {
        saveState(0);
    }
    if (mMemory->mInput->isKeyPressed(GBK_l))
    {
        loadState(0);
    }

    if (mMemory->mInput->isKeyPressed(GBK_KP_PLUS)) if (fpsSpeed < 5)
        fpsSpeed++;

    if (mMemory->mInput->isKeyPressed(GBK_KP_MINUS)) if (fpsSpeed > 1)
        fpsSpeed--;

    if (!mMemory->mInput->isKeyPressed(GBK_SPACE))
    {
        if (time1 > SDL_GetTicks())
        {
            SDL_Delay(time1 - SDL_GetTicks());
        }
        time1 = SDL_GetTicks() + ((17 / fpsSpeed));
    }
}

void cCpu::updateIMEFlag() {
    if (intStatus == 3) {
        //Turn on interrupts
        interruptsEnabled = true;
        intStatus = 0;
    }
    if (intStatus == 4) {
        //Turn off interrupts
        interruptsEnabled = false;
        intStatus = 0;
    }
    if (intStatus == 1)//Turn on in the next opcode
        intStatus = 3;
    if (intStatus == 2)//Turn off in the next opcode
        intStatus = 4;
}

void cCpu::executeOpCode(int a_opCode) {
    switch (a_opCode) {
        case 0x06: b = readNextByte();
            break;
        case 0x0E: c = readNextByte();
            break;
        case 0x16: d = readNextByte();
            break;
        case 0x1E: e = readNextByte();
            break;
        case 0x26: h = readNextByte();
            break;
        case 0x2E: l = readNextByte();
            break;

        case 0x7F: a = a;
            break;
        case 0x78: a = b;
            break;
        case 0x79: a = c;
            break;
        case 0x7A: a = d;
            break;
        case 0x7B: a = e;
            break;
        case 0x7C: a = h;
            break;
        case 0x7D: a = l;
            break;
        case 0x7E: a = mMemory->readByte(hl());
            break;

        case 0x40: b = b;
            break;
        case 0x41: b = c;
            break;
        case 0x42: b = d;
            break;
        case 0x43: b = e;
            break;
        case 0x44: b = h;
            break;
        case 0x45: b = l;
            break;
        case 0x46: b = mMemory->readByte(hl());
            break;

        case 0x48: c = b;
            break;
        case 0x49: c = c;
            break;
        case 0x4A: c = d;
            break;
        case 0x4B: c = e;
            break;
        case 0x4C: c = h;
            break;
        case 0x4D: c = l;
            break;
        case 0x4E: c = mMemory->readByte(hl());
            break;

        case 0x50: d = b;
            break;
        case 0x51: d = c;
            break;
        case 0x52: d = d;
            break;
        case 0x53: d = e;
            break;
        case 0x54: d = h;
            break;
        case 0x55: d = l;
            break;
        case 0x56: d = mMemory->readByte(hl());
            break;

        case 0x58: e = b;
            break;
        case 0x59: e = c;
            break;
        case 0x5A: e = d;
            break;
        case 0x5B: e = e;
            break;
        case 0x5C: e = h;
            break;
        case 0x5D: e = l;
            break;
        case 0x5E: e = mMemory->readByte(hl());
            break;

        case 0x60: h = b;
            break;
        case 0x61: h = c;
            break;
        case 0x62: h = d;
            break;
        case 0x63: h = e;
            break;
        case 0x64: h = h;
            break;
        case 0x65: h = l;
            break;
        case 0x66: h = mMemory->readByte(hl());
            break;

        case 0x68: l = b;
            break;
        case 0x69: l = c;
            break;
        case 0x6A: l = d;
            break;
        case 0x6B: l = e;
            break;
        case 0x6C: l = h;
            break;
        case 0x6D: l = l;
            break;
        case 0x6E: l = mMemory->readByte(hl());
            break;

        case 0x70: mMemory->writeByte(hl(), b);
            break;
        case 0x71: mMemory->writeByte(hl(), c);
            break;
        case 0x72: mMemory->writeByte(hl(), d);
            break;
        case 0x73: mMemory->writeByte(hl(), e);
            break;
        case 0x74: mMemory->writeByte(hl(), h);
            break;
        case 0x75: mMemory->writeByte(hl(), l);
            break;
        case 0x36: mMemory->writeByte(hl(), readNextByte());
            break;

        case 0x0A: a = mMemory->readByte(bc());
            break;
        case 0x1A: a = mMemory->readByte(de());
            break;
        case 0xFA: a = mMemory->readByte(readNextWord());
            break;
        case 0x3E: a = readNextByte();
            break;

        case 0x47: b = a;
            break;
        case 0x4F: c = a;
            break;
        case 0x57: d = a;
            break;
        case 0x5F: e = a;
            break;
        case 0x67: h = a;
            break;
        case 0x6F: l = a;
            break;
        case 0x02: mMemory->writeByte(bc(), a);
            break;
        case 0x12: mMemory->writeByte(de(), a);
            break;
        case 0x77: mMemory->writeByte(hl(), a);
            break;
        case 0xEA: mMemory->writeByte(readNextWord(), a);
            break;

        case 0xF2: a = mMemory->readByte(0xFF00 | c);
            break;
        case 0xE2: mMemory->writeByte(0xFF00 | c, a);
            break;

        case 0x3A: a = mMemory->readByte(hl());
            hl(hl() - 1);
            break;
        case 0x32: mMemory->writeByte(hl(), a);
            hl(hl() - 1);
            break;
        case 0x2A: a = mMemory->readByte(hl());
            hl(hl() + 1);
            break;
        case 0x22: mMemory->writeByte(hl(), a);
            hl(hl() + 1);
            break;

        case 0xE0: mMemory->writeByte(0xFF00 | readNextByte(), a);
            break;
        case 0xF0: a = mMemory->readByte(0xFF00 | readNextByte());
            break;

        case 0x01: bc(readNextWord());
            break;
        case 0x11: de(readNextWord());
            break;
        case 0x21: hl(readNextWord());
            break;
        case 0x31: sp = readNextWord();
            break;

        case 0xF9: sp = hl();
            break;
        case 0xF8: ldhl(readNextByte());
            break;
        case 0x08: ldnnsp(readNextWord());
            break;

        case 0xF5: push(af());
            break;
        case 0xC5: push(bc());
            break;
        case 0xD5: push(de());
            break;
        case 0xE5: push(hl());
            break;

        case 0xF1: popaf();
            break;
        case 0xC1: pop(b, c);
            break;
        case 0xD1: pop(d, e);
            break;
        case 0xE1: pop(h, l);
            break;

        case 0x87: add(a);
            break;
        case 0x80: add(b);
            break;
        case 0x81: add(c);
            break;
        case 0x82: add(d);
            break;
        case 0x83: add(e);
            break;
        case 0x84: add(h);
            break;
        case 0x85: add(l);
            break;
        case 0x86: add(mMemory->readByte(hl()));
            break;
        case 0xC6: add(readNextByte());
            break;

        case 0x8F: adc(a);
            break;
        case 0x88: adc(b);
            break;
        case 0x89: adc(c);
            break;
        case 0x8A: adc(d);
            break;
        case 0x8B: adc(e);
            break;
        case 0x8C: adc(h);
            break;
        case 0x8D: adc(l);
            break;
        case 0x8E: adc(mMemory->readByte(hl()));
            break;
        case 0xCE: adc(readNextByte());
            break;

        case 0x97: sub(a);
            break;
        case 0x90: sub(b);
            break;
        case 0x91: sub(c);
            break;
        case 0x92: sub(d);
            break;
        case 0x93: sub(e);
            break;
        case 0x94: sub(h);
            break;
        case 0x95: sub(l);
            break;
        case 0x96: sub(mMemory->readByte(hl()));
            break;
        case 0xD6: sub(readNextByte());
            break;

        case 0x9F: sbc(a);
            break;
        case 0x98: sbc(b);
            break;
        case 0x99: sbc(c);
            break;
        case 0x9A: sbc(d);
            break;
        case 0x9B: sbc(e);
            break;
        case 0x9C: sbc(h);
            break;
        case 0x9D: sbc(l);
            break;
        case 0x9E: sbc(mMemory->readByte(hl()));
            break;
        case 0xDE: sbc(readNextByte());
            break;

        case 0xA7: z8and(a);
            break;
        case 0xA0: z8and(b);
            break;
        case 0xA1: z8and(c);
            break;
        case 0xA2: z8and(d);
            break;
        case 0xA3: z8and(e);
            break;
        case 0xA4: z8and(h);
            break;
        case 0xA5: z8and(l);
            break;
        case 0xA6: z8and(mMemory->readByte(hl()));
            break;
        case 0xE6: z8and(readNextByte());
            break;

        case 0xB7: z8or(a);
            break;
        case 0xB0: z8or(b);
            break;
        case 0xB1: z8or(c);
            break;
        case 0xB2: z8or(d);
            break;
        case 0xB3: z8or(e);
            break;
        case 0xB4: z8or(h);
            break;
        case 0xB5: z8or(l);
            break;
        case 0xB6: z8or(mMemory->readByte(hl()));
            break;
        case 0xF6: z8or(readNextByte());
            break;

        case 0xAF: z8xor(a);
            break;
        case 0xA8: z8xor(b);
            break;
        case 0xA9: z8xor(c);
            break;
        case 0xAA: z8xor(d);
            break;
        case 0xAB: z8xor(e);
            break;
        case 0xAC: z8xor(h);
            break;
        case 0xAD: z8xor(l);
            break;
        case 0xAE: z8xor(mMemory->readByte(hl()));
            break;
        case 0xEE: z8xor(readNextByte());
            break;

        case 0xBF: cp(a);
            break;
        case 0xB8: cp(b);
            break;
        case 0xB9: cp(c);
            break;
        case 0xBA: cp(d);
            break;
        case 0xBB: cp(e);
            break;
        case 0xBC: cp(h);
            break;
        case 0xBD: cp(l);
            break;
        case 0xBE: cp(mMemory->readByte(hl()));
            break;
        case 0xFE: cp(readNextByte());
            break;

        case 0x3C: inc(a);
            break;
        case 0x04: inc(b);
            break;
        case 0x0C: inc(c);
            break;
        case 0x14: inc(d);
            break;
        case 0x1C: inc(e);
            break;
        case 0x24: inc(h);
            break;
        case 0x2C: inc(l);
            break;
        case 0x34: inchl();
            break;

        case 0x3D: dec(a);
            break;
        case 0x05: dec(b);
            break;
        case 0x0D: dec(c);
            break;
        case 0x15: dec(d);
            break;
        case 0x1D: dec(e);
            break;
        case 0x25: dec(h);
            break;
        case 0x2D: dec(l);
            break;
        case 0x35: dechl();
            break;

        case 0x09: addhl(bc());
            break;
        case 0x19: addhl(de());
            break;
        case 0x29: addhl(hl());
            break;
        case 0x39: addhl(sp);
            break;
        case 0xE8: addsp(readNextByte());
            break;

        case 0x03: inc(b, c);
            break;
        case 0x13: inc(d, e);
            break;
        case 0x23: inc(h, l);
            break;
        case 0x33: sp++;
            break;

        case 0x0B: dec(b, c);
            break;
        case 0x1B: dec(d, e);
            break;
        case 0x2B: dec(h, l);
            break;
        case 0x3B: sp--;
            break;

        case 0x27: daa();
            break;
        case 0x2F: a = ~a;
            n_flag = true;
            h_flag = true;
            break;
        case 0x3F: c_flag = (c_flag == false);
            n_flag = false;
            h_flag = false;
            break;
        case 0x37: c_flag = true;
            n_flag = false;
            h_flag = false;
            break;

        case 0x00:
            break;

        case 0x76: // HALT
            if (mMemory->mInterrupts->getReadyInterrupts() == 0)
                pc--;
            break;
        case 0x10:
            if (isColor)
                mCurrentSpeed = mMemory->changeSpeed();
            break;
        case 0xF3: intStatus = 2;
            break;
        case 0xFB: intStatus = 1;
            break;

        case 0x07: rlca();
            break;
        case 0x17: rla();
            break;
        case 0x1F: rra();
            break;
        case 0x0F: rrca();
            break;

        case 0xC3: pc = readNextWord();
            break;
        case 0xC2: jp(z_flag == false, readNextWord());
            break;
        case 0xCA: jp(z_flag == true, readNextWord());
            break;
        case 0xD2: jp(c_flag == false, readNextWord());
            break;
        case 0xDA: jp(c_flag == true, readNextWord());
            break;
        case 0xE9: pc = hl();
            break;

        case 0x18: jr(true, readNextByte());
            break;
        case 0x20: jr(z_flag == false, readNextByte());
            break;
        case 0x28: jr(z_flag == true, readNextByte());
            break;
        case 0x30: jr(c_flag == false, readNextByte());
            break;
        case 0x38: jr(c_flag == true, readNextByte());
            break;

        case 0xCD: call(true, readNextWord());
            break;
        case 0xC4: call(z_flag == false, readNextWord());
            break;
        case 0xCC: call(z_flag == true, readNextWord());
            break;
        case 0xD4: call(c_flag == false, readNextWord());
            break;
        case 0xDC: call(c_flag == true, readNextWord());
            break;

        case 0xC7: rst(0x00);
            break;
        case 0xCF: rst(0x08);
            break;
        case 0xD7: rst(0x10);
            break;
        case 0xDF: rst(0x18);
            break;
        case 0xE7: rst(0x20);
            break;
        case 0xEF: rst(0x28);
            break;
        case 0xF7: rst(0x30);
            break;
        case 0xFF: rst(0x38);
            break;

        case 0xC9: ret(true);
            break;
        case 0xC0: ret(z_flag == false);
            break;
        case 0xC8: ret(z_flag == true);
            break;
        case 0xD0: ret(c_flag == false);
            break;
        case 0xD8: ret(c_flag == true);
            break;

        case 0xD9: ret(true);
            interruptsEnabled = true;
            break;

        default: //TODO: Raise exception
            WARNING(true, "Unkown opcode");
    }
}

void cCpu::executeCBOpCode(u8 a_cbOpCode) {
    u8 innerBits = 0;
    switch (a_cbOpCode) {
        case 0x37: swap(a);
            break;
        case 0x30: swap(b);
            break;
        case 0x31: swap(c);
            break;
        case 0x32: swap(d);
            break;
        case 0x33: swap(e);
            break;
        case 0x34: swap(h);
            break;
        case 0x35: swap(l);
            break;
        case 0x36: swaphl();
            break;

        case 0x07: rlc(a);
            break;
        case 0x00: rlc(b);
            break;
        case 0x01: rlc(c);
            break;
        case 0x02: rlc(d);
            break;
        case 0x03: rlc(e);
            break;
        case 0x04: rlc(h);
            break;
        case 0x05: rlc(l);
            break;
        case 0x06: rlchl();
            break;

        case 0x17: rl(a);
            break;
        case 0x10: rl(b);
            break;
        case 0x11: rl(c);
            break;
        case 0x12: rl(d);
            break;
        case 0x13: rl(e);
            break;
        case 0x14: rl(h);
            break;
        case 0x15: rl(l);
            break;
        case 0x16: rlhl();
            break;

        case 0x0F: rrc(a);
            break;
        case 0x08: rrc(b);
            break;
        case 0x09: rrc(c);
            break;
        case 0x0A: rrc(d);
            break;
        case 0x0B: rrc(e);
            break;
        case 0x0C: rrc(h);
            break;
        case 0x0D: rrc(l);
            break;
        case 0x0E: rrchl();
            break;

        case 0x1F: rr(a);
            break;
        case 0x18: rr(b);
            break;
        case 0x19: rr(c);
            break;
        case 0x1A: rr(d);
            break;
        case 0x1B: rr(e);
            break;
        case 0x1C: rr(h);
            break;
        case 0x1D: rr(l);
            break;
        case 0x1E: rrhl();
            break;

        case 0x27: sla(a);
            break;
        case 0x20: sla(b);
            break;
        case 0x21: sla(c);
            break;
        case 0x22: sla(d);
            break;
        case 0x23: sla(e);
            break;
        case 0x24: sla(h);
            break;
        case 0x25: sla(l);
            break;
        case 0x26: slahl();
            break;

        case 0x2F: sra(a);
            break;
        case 0x28: sra(b);
            break;
        case 0x29: sra(c);
            break;
        case 0x2A: sra(d);
            break;
        case 0x2B: sra(e);
            break;
        case 0x2C: sra(h);
            break;
        case 0x2D: sra(l);
            break;
        case 0x2E: srahl();
            break;

        case 0x3F: srl(a);
            break;
        case 0x38: srl(b);
            break;
        case 0x39: srl(c);
            break;
        case 0x3A: srl(d);
            break;
        case 0x3B: srl(e);
            break;
        case 0x3C: srl(h);
            break;
        case 0x3D: srl(l);
            break;
        case 0x3E: srlhl();
            break;

        default:
            innerBits = a_cbOpCode & 0x38;
            switch (a_cbOpCode & 0xC7) { //Discard the bit number
                case 0x47: bit(innerBits, a);
                    break;
                case 0x40: bit(innerBits, b);
                    break;
                case 0x41: bit(innerBits, c);
                    break;
                case 0x42: bit(innerBits, d);
                    break;
                case 0x43: bit(innerBits, e);
                    break;
                case 0x44: bit(innerBits, h);
                    break;
                case 0x45: bit(innerBits, l);
                    break;
                case 0x46: bit(innerBits, mMemory->readByte(hl()));
                    break;

                case 0xC7: set(innerBits, a);
                    break;
                case 0xC0: set(innerBits, b);
                    break;
                case 0xC1: set(innerBits, c);
                    break;
                case 0xC2: set(innerBits, d);
                    break;
                case 0xC3: set(innerBits, e);
                    break;
                case 0xC4: set(innerBits, h);
                    break;
                case 0xC5: set(innerBits, l);
                    break;
                case 0xC6: sethl(innerBits);
                    break;

                case 0x87: res(innerBits, a);
                    break;
                case 0x80: res(innerBits, b);
                    break;
                case 0x81: res(innerBits, c);
                    break;
                case 0x82: res(innerBits, d);
                    break;
                case 0x83: res(innerBits, e);
                    break;
                case 0x84: res(innerBits, h);
                    break;
                case 0x85: res(innerBits, l);
                    break;
                case 0x86: reshl(innerBits);
                    break;
                default: // TODO: Raise exception.
                    std::cout << "Opcode " << std::hex << a_cbOpCode << "no ejecutado" << std::endl;
            }
    }
}

void cCpu::adc(u8 value) {
    u8 result = a + value + (u8) c_flag;
    z_flag = (result & 0xFFu) == 0;
    n_flag = false;
    h_flag = ((a & 0xFu) + (value & 0xFu) + (u8) c_flag) > 0xFu;
    c_flag = (a + value + (u8) c_flag) > 0xFFu;
    a = result;
}

void cCpu::add(u8 value) {
    z_flag = ((a + value) & 0xFFu) == 0;
    n_flag = false;
    h_flag = (a & 0xFu) + (value & 0xFu) > 0xFu;
    c_flag = (a + value) > 0xFFu;
    a += value;
}

void cCpu::addhl(u16 value) {
    u16 hl_value = hl();
    n_flag = false;
    h_flag = ((hl_value & 0xFFFu) + (value & 0xFFFu)) > 0xFFFu;
    //h_flag=((hl_value>>8)+(value>>8))>0xFF;
    c_flag = (hl_value + value) > 0xFFFFu;
    hl(hl_value + value);
}

void cCpu::addsp(s8 value) {
    c_flag = ((sp & 0xFFu) + (u8) value) > 0xFFu;
    h_flag = ((sp & 0xFu) + ((u8) value & 0xFu)) > 0xFu;
    sp += value;
    z_flag = false;
    n_flag = false;
}

void cCpu::z8and(u8 value)
{
    a &= value;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = true;
    c_flag = false;
}

void cCpu::bit(u8 bit_number, u8 register_value) {
    bit_number >>= 3u;
    z_flag = (register_value & (1u << bit_number)) == 0;
    n_flag = false;
    h_flag = true;
}

void cCpu::call(bool condition, u16 address) {
    if (condition) {
        push(pc);
        pc = address;
        mCyclesSum += 12;
    }
}

void cCpu::cp(u8 val) {
    z_flag = (a == val);
    n_flag = true;
    h_flag = (a & 0xFu) < (val & 0xFu);
    c_flag = a < val;
}

void cCpu::daa() {
    int result = a;
    if (n_flag) {
        if (h_flag) result = (result - 6) & 0xFF;
        if (c_flag) result -= 0x60;
    } else {
        if (h_flag || (result & 0xF) > 9) result += 6;
        if (c_flag || result > 0x9F) result += 0x60;

    }

    if ((result & 0x100) == 0x100)
        c_flag = true;
    a = result & 0xFF;
    z_flag = (a == 0);
    h_flag = false;
}

void cCpu::dec(u8 &reg) {
    z_flag = (reg == 1);
    n_flag = true;
    h_flag = (reg & 0xFu) == 0;
    reg -= 1;
}

void cCpu::dec(u8 &high_register, u8 &low_register) {
    u16 result = (high_register << 8u) | low_register;
    result -= 1;
    high_register = result >> 8u;
    low_register = result & 0xFFu;
}

void cCpu::dechl() {
    u8 result = mMemory->readByte(hl());
    dec(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::inc(u8 &reg) {
    z_flag = (reg == 0xFF);
    n_flag = false;
    h_flag = (reg & 0xFu) == 0xF;
    reg += 1;
}

void cCpu::inc(u8 &high_register, u8 &low_register) {
    u16 result = (high_register << 8) | low_register;
    result += 1;
    high_register = result >> 8;
    low_register = result & 0xFF;
}

void cCpu::inchl() {
    u8 result = mMemory->readByte(hl());
    inc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::jp(bool condition, u16 address) {
    if (condition) {
        pc = address;
        mCyclesSum += 4;
    }
}

void cCpu::jr(bool condition, s8 value) {
    if (condition) {
        pc += value;
        mCyclesSum += 20;
    }
}

void cCpu::ldhl(s8 value) {

    c_flag = ((sp & 0xFFu) + (u8) value > 0xFFu);
    h_flag = ((sp & 0xFu) + ((u8) value & 0xFu) > 0xFu);
    z_flag = false;
    n_flag = false;
    hl(sp + value);
}

void cCpu::ldnnsp(u16 value) {
    mMemory->writeByte(value, sp & 0xFFu);
    mMemory->writeByte(value + 1, sp >> 8u);
}

void cCpu::z8or(u8 value) {
    a |= value;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}

void cCpu::pop(u8 &high_register, u8 &low_register) {
    low_register = mMemory->readByte(sp);
    sp++;
    high_register = mMemory->readByte(sp);
    sp++;

}

void cCpu::popaf() {
    u8 flags_value;
    pop(a, flags_value);
    flags(flags_value);
}

void cCpu::push(u16 regs) {
    sp--;
    mMemory->writeByte(sp, regs >> 8);
    sp--;
    mMemory->writeByte(sp, regs & 0xFF);
}

void cCpu::res(u8 bit_number, u8 &reg) {
    reg &= ~(1u << (bit_number >> 3u));
}

void cCpu::reshl(u8 bit_number) {
    u8 result = mMemory->readByte(hl());
    res(bit_number, result);
    mMemory->writeByte(hl(), result);
}

void cCpu::ret(bool condition) {
    u8 high_byte, low_byte;
    if (condition) {
        pop(high_byte, low_byte);
        pc = (high_byte << 8) | low_byte;
    }
}

void cCpu::rlc(u8 &reg) {
    c_flag = (reg >> 7) & 1;
    reg = (reg << 1) | c_flag;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rlca() {
    c_flag = (a >> 7) & 1;
    a = (a << 1) | c_flag;
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rlchl() {
    u8 result = mMemory->readByte(hl());
    rlc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rl(u8 &reg) {
    u8 carry_flag_bit = (u8) c_flag;
    c_flag = (reg >> 7u) & 1u;
    reg = (reg << 1u) | carry_flag_bit;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rla() {
    u8 result = a >> 7;
    a = (a << 1) | c_flag;
    z_flag = false;
    c_flag = result & 1;
    n_flag = false;
    h_flag = false;
}

void cCpu::rlhl() {
    u8 result = mMemory->readByte(hl());
    rl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rrc(u8 &reg) {
    c_flag = reg & 1u;
    reg = (reg >> 1u) | (c_flag << 7u);
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rrca() {
    c_flag = a & 1u;
    a = (a >> 1u) | (c_flag << 7u);
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rrchl() {
    u8 result = mMemory->readByte(hl());
    rrc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rr(u8 &reg) {
    bool new_carry_flag = reg & 1u; //Keep the first bit
    reg = (reg >> 1u) | (c_flag << 7u); //Shift n right and put carry at the top
    c_flag = new_carry_flag; //Put the kept n bit into carry
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rra() {
    bool result = a & 1u; //Keep the first bit
    a = (a >> 1u) | (c_flag << 7u); //Shift n right and put carry at the top
    c_flag = result; //Put the kept n bit into carry
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rrhl() {
    u8 result = mMemory->readByte(hl());
    rr(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rst(u8 address) {
    call(true, address);
}

void cCpu::sbc(u8 value) {
    u16 result = a - value - c_flag;
    n_flag = true;
    //h_flag = (result & 0xF) > (a & 0xF);
    c_flag = (result > a);
    h_flag = ((a ^ value ^ (result & 0xFFu)) & 0x10u) != 0;

    a = result & 0xFFu;
    z_flag = (a == 0);
}

void cCpu::set(u8 bit, u8 &reg) {
    reg |= (1u << (bit >> 3u));
}

void cCpu::sethl(u8 bit_number) {
    u8 result = mMemory->readByte(hl());
    set(bit_number, result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sla(u8 &reg) {
    c_flag = (reg >> 7u) & 1u;
    reg <<= 1u;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::slahl() {
    u8 result = mMemory->readByte(hl());
    sla(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sra(u8 &reg) {
    c_flag = reg & 1u;
    reg = (reg >> 1u) | (reg & 0x80u);
    z_flag = (reg == 0);
    h_flag = false;
    n_flag = false;
}

void cCpu::srahl() {
    u8 result = mMemory->readByte(hl());
    sra(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::srl(u8 &reg) {
    c_flag = reg & 1u;
    reg >>= 1u;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::srlhl() {
    u8 result = mMemory->readByte(hl());
    srl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sub(u8 value) {
    z_flag = (a == value);
    n_flag = true;
    h_flag = (value & 0xFu) > (a & 0xFu);
    c_flag = (value > a);
    a -= value;
}

void cCpu::swap(u8 &reg) {
    reg = ((reg & 0xFu) << 4u) | (reg >> 4u);
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}

void cCpu::swaphl() {
    u8 result = mMemory->readByte(hl());
    swap(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::z8xor(u8 val) {
    a ^= val;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}

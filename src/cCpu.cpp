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

u8 cCpu::flags()
{
    return ((zf << 7) | (nf << 6) | (hf << 5) | (cf << 4));
}

void cCpu::flags(u8 val)
{
    zf = ((val >> 7) & 1);
    nf = ((val >> 6) & 1);
    hf = ((val >> 5) & 1);
    cf = ((val >> 4) & 1);
}

u16 cCpu::af()
{
    return (a << 8) | flags();
}

void cCpu::af(u16 val)
{
    a = val >> 8;
    flags(val & 0xFF);
}

u16 cCpu::bc()
{
    return ((b << 8) | c);
}

void cCpu::bc(u16 val)
{
    b = val >> 8;
    c = val & 0xFF;
}

u16 cCpu::de()
{
    return ((d << 8) | e);
}

void cCpu::de(u16 val)
{
    d = val >> 8;
    e = val & 0xFF;
}

u16 cCpu::hl()
{
    return ((h << 8) | l);
}

void cCpu::hl(u16 val)
{
    h = val >> 8;
    l = val & 0xFF;
}

u8 cCpu::readNextByte()
{
    return mMemory->readByte(pc++);
}

u16 cCpu::readNextWord()
{
    return (mMemory->readByte(pc++) | (mMemory->readByte(pc++) << 8));
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
    int opcode = fetchOpcode();
    executeOpcode(opcode);
    updateCycles(opcode);
    checkInterrupts();
    updateModes();


    if (rtcCount >= 4194304)
    {
        rtcCount -= 4194304;
        mMemory->rtcCounter();
    }
}

int cCpu::fetchOpcode()
{
    return mMemory->readByte(pc++);
}

void cCpu::updateCycles(int a_opcode)
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
    if (lyCycles <= 0)
    {
        lyCycles += (456 << mCurrentSpeed);
        scanLine = ++mMemory->IOMap[0xFF44][0]; //Increment LY
        if (scanLine == 153)
        {
            scanLine = 0;
            mMemory->IOMap[0xFF44][0] = 0;
        }

        if (scanLine == mMemory->IOMap[0xFF45][0])//We have a LY==LYC interrupt
        {
            if ((mMemory->IOMap[0xFF41][0] & 0x40) && mMemory->IOMap[0xFF40][0] & 0x80)
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
                if ((mMemory->IOMap[0xFF41][0] & 8) && (mMemory->IOMap[0xFF40][0] & 0x80))
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
                if (mMemory->IOMap[0xFF40][0] & 0x80)
                    mMemory->mInterrupts->setInterrupt(cInterrupts::VBLANK);
                if (mMemory->IOMap[0xFF41][0] & 0x10 && mMemory->IOMap[0xFF40][0] & 0x80)
                    mMemory->mInterrupts->setInterrupt(cInterrupts::LCDC); //Mode 1 V-Blank LCDC Interrupt
                break;

            case 2://Do Mode 2 actions
                cyclesCount += (80 << mCurrentSpeed);
                nextMode = 3;
                setMode(2);
                if (mMemory->IOMap[0xFF41][0] & 0x20 && mMemory->IOMap[0xFF40][0] >> 7 == 1)
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

void cCpu::executeOpcode(int a_opcode)
{
    int cbOpcode;
    mCyclesSum += mOpcodeCyclesTable[a_opcode];
    if (intStatus == 3)//Turn on interrupts
    {
        interruptsEnabled = true;
        intStatus = 0;
    }
    if (intStatus == 4)//Turn off interrupts
    {
        interruptsEnabled = false;
        intStatus = 0;
    }
    if (intStatus == 1)//Turn on in the next opcode
        intStatus = 3;
    if (intStatus == 2)//Turn off in the next opcode
        intStatus = 4;

    switch (a_opcode)
    {
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
            nf = true;
            hf = true;
            break;
        case 0x3F: cf = (cf == false);
            nf = false;
            hf = false;
            break;
        case 0x37: cf = true;
            nf = false;
            hf = false;
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

        case 0xCB://CBOpcodes
            cbOpcode = mMemory->readByte(pc++);
            mCyclesSum += mCBOpcodeCyclesTable[cbOpcode];
            switch (cbOpcode)
            {
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
                    switch (cbOpcode & 199)//Discard the bit number
                    {
                        case 0x47: bit(cbOpcode & 0x38, a);
                            break;
                        case 0x40: bit(cbOpcode & 0x38, b);
                            break;
                        case 0x41: bit(cbOpcode & 0x38, c);
                            break;
                        case 0x42: bit(cbOpcode & 0x38, d);
                            break;
                        case 0x43: bit(cbOpcode & 0x38, e);
                            break;
                        case 0x44: bit(cbOpcode & 0x38, h);
                            break;
                        case 0x45: bit(cbOpcode & 0x38, l);
                            break;
                        case 0x46: bit(cbOpcode & 0x38, mMemory->readByte(hl()));
                            break;

                        case 0xC7: set(cbOpcode & 0x38, a);
                            break;
                        case 0xC0: set(cbOpcode & 0x38, b);
                            break;
                        case 0xC1: set(cbOpcode & 0x38, c);
                            break;
                        case 0xC2: set(cbOpcode & 0x38, d);
                            break;
                        case 0xC3: set(cbOpcode & 0x38, e);
                            break;
                        case 0xC4: set(cbOpcode & 0x38, h);
                            break;
                        case 0xC5: set(cbOpcode & 0x38, l);
                            break;
                        case 0xC6: sethl(cbOpcode & 0x38);
                            break;

                        case 0x87: res(cbOpcode & 0x38, a);
                            break;
                        case 0x80: res(cbOpcode & 0x38, b);
                            break;
                        case 0x81: res(cbOpcode & 0x38, c);
                            break;
                        case 0x82: res(cbOpcode & 0x38, d);
                            break;
                        case 0x83: res(cbOpcode & 0x38, e);
                            break;
                        case 0x84: res(cbOpcode & 0x38, h);
                            break;
                        case 0x85: res(cbOpcode & 0x38, l);
                            break;
                        case 0x86: reshl(cbOpcode & 0x38);
                            break;
                        default: // TODO: Raise exception.
                            std::cout << "Opcode " << std::hex << cbOpcode << "no ejecutado" << std::endl;
                    }
            }
            break;

        case 0xC3: pc = readNextWord();
            break;
        case 0xC2: jp(zf == false, readNextWord());
            break;
        case 0xCA: jp(zf == true, readNextWord());
            break;
        case 0xD2: jp(cf == false, readNextWord());
            break;
        case 0xDA: jp(cf == true, readNextWord());
            break;
        case 0xE9: pc = hl();
            break;

        case 0x18: jr(true, readNextByte());
            break;
        case 0x20: jr(zf == false, readNextByte());
            break;
        case 0x28: jr(zf == true, readNextByte());
            break;
        case 0x30: jr(cf == false, readNextByte());
            break;
        case 0x38: jr(cf == true, readNextByte());
            break;

        case 0xCD: call(true, readNextWord());
            break;
        case 0xC4: call(zf == false, readNextWord());
            break;
        case 0xCC: call(zf == true, readNextWord());
            break;
        case 0xD4: call(cf == false, readNextWord());
            break;
        case 0xDC: call(cf == true, readNextWord());
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
        case 0xC0: ret(zf == false);
            break;
        case 0xC8: ret(zf == true);
            break;
        case 0xD0: ret(cf == false);
            break;
        case 0xD8: ret(cf == true);
            break;

        case 0xD9: ret(true);
            interruptsEnabled = true;
            break;

        default: //TODO: Raise exception
            WARNING(true, "Unkown opcode");
    }
}

void cCpu::adc(u8 val)
{
    u8 t = a + val + (u8) cf;
    zf = ((t & 0xFF) == 0);
    nf = false;
    hf = (((a & 0xF) + (val & 0xF) + (u8) cf) > 0xF);
    cf = ((a + val + (u8) cf) > 0xFF);
    a = t;
}

void cCpu::add(u8 val)
{
    zf = (((a + val) & 0xFF) == 0);
    nf = false;
    hf = ((a & 0xF) + (val & 0xF) > 0xF);
    cf = ((a + val) > 0xFF);
    a += val;
}

void cCpu::addhl(u16 val)
{
    u16 temp = hl();
    nf = false;
    hf = ((temp & 0xFFF) + (val & 0xFFF)) > 0xFFF;
    //hf=((temp>>8)+(val>>8))>0xFF;
    cf = (temp + val) > 0xFFFF;
    hl(temp + val);
}

void cCpu::addsp(s8 val)
{
    cf = ((sp & 0xFF) + (u8) val) > 0xFF;
    hf = ((sp & 0xF) + ((u8) val & 0xF)) > 0xF;
    sp += val;
    zf = false;
    nf = false;
}

void cCpu::z8and(u8 val)
{
    a &= val;
    zf = (a == 0);
    nf = false;
    hf = true;
    cf = false;
}

void cCpu::bit(u8 bit, u8 reg)
{
    bit >>= 3;
    WARNING(bit > 7, "Bit es mayor a 7");
    zf = (reg & (1 << bit)) == 0;
    nf = false;
    hf = true;
}

void cCpu::call(bool condition, u16 address)
{
    if (condition)
    {
        push(pc);
        pc = address;
        mCyclesSum += 12;
    }
}

void cCpu::cp(u8 val)
{
    zf = (a == val);
    nf = true;
    hf = (a & 0xF) < (val & 0xF);
    cf = a < val;
}

void cCpu::daa(void)
{
    int temp = a;
    if (nf)
    {
        if (hf) temp = (temp - 6) & 0xFF;
        if (cf) temp -= 0x60;
    }
    else
    {
        if (hf || (temp & 0xF) > 9) temp += 6;
        if (cf || temp > 0x9F) temp += 0x60;

    }

    if ((temp & 0x100) == 0x100)
        cf = true;
    a = temp & 0xFF;
    zf = (a == 0);
    hf = false;
}

void cCpu::dec(u8 &reg)
{
    zf = (reg == 1);
    nf = true;
    hf = (reg & 0xF) == 0;
    reg -= 1;
}

void cCpu::dec(u8 &r1, u8 &r2)
{
    u16 reg = (r1 << 8) | r2;
    reg -= 1;
    r1 = reg >> 8;
    r2 = reg & 0xFF;
}

void cCpu::dechl(void)
{
    WARNING(hl() < 0x4000, "dechl??");
    u8 temp = mMemory->readByte(hl());
    dec(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::inc(u8 &reg)
{
    zf = (reg == 0xFF);
    nf = false;
    hf = (reg & 0xF) == 0xF;
    reg += 1;
}

void cCpu::inc(u8 &r1, u8 &r2)
{
    u16 reg = (r1 << 8) | r2;
    reg += 1;
    r1 = reg >> 8;
    r2 = reg & 0xFF;
}

void cCpu::inchl(void)
{
    WARNING(hl() < 0x4000, "inchl??");
    u8 temp = mMemory->readByte(hl());
    inc(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::jp(bool condition, u16 address)
{
    if (condition)
    {
        pc = address;
        mCyclesSum += 4;
    }
}

void cCpu::jr(bool condition, s8 val)
{
    if (condition)
    {
        pc += val;
        mCyclesSum += 20;
    }
}

void cCpu::ldhl(s8 val)
{

    cf = ((sp & 0xFF) + (u8) val > 0xFF);
    hf = ((sp & 0xF) + ((u8) val & 0xF) > 0xF);
    zf = false;
    nf = false;
    hl(sp + val);
}

void cCpu::ldnnsp(u16 val)
{
    mMemory->writeByte(val, sp & 0xFF);
    mMemory->writeByte(val + 1, sp >> 8);
}

void cCpu::z8or(u8 val)
{
    a |= val;
    zf = (a == 0);
    nf = false;
    hf = false;
    cf = false;
}

void cCpu::pop(u8 &r1, u8 &r2)
{
    r2 = mMemory->readByte(sp);
    sp++;
    r1 = mMemory->readByte(sp);
    sp++;

}

void cCpu::popaf(void)
{
    u8 temp;
    pop(a, temp);
    flags(temp);
}

void cCpu::push(u16 regs)
{
    sp--;
    mMemory->writeByte(sp, regs >> 8);
    sp--;
    mMemory->writeByte(sp, regs & 0xFF);
}

void cCpu::res(u8 bit, u8 &reg)
{
    reg &= ~(1 << (bit >> 3));
}

void cCpu::reshl(u8 bit)
{
    WARNING(hl() < 0x4000, "reshl??");
    u8 temp = mMemory->readByte(hl());
    res(bit, temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::ret(bool condition)
{
    u8 t1, t2;
    if (condition)
    {
        pop(t1, t2);
        pc = (t1 << 8) | t2;
    }
}

void cCpu::rlc(u8 &reg)
{
    cf = (reg >> 7) & 1;
    reg = (reg << 1) | cf;
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::rlca(void)
{
    cf = (a >> 7) & 1;
    a = (a << 1) | cf;
    zf = false;
    nf = false;
    hf = false;
}

void cCpu::rlchl(void)
{
    WARNING(hl() < 0x4000, "rlchl??");
    u8 temp = mMemory->readByte(hl());
    rlc(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::rl(u8 &reg)
{
    u8 temp = (u8) cf;
    cf = (reg >> 7) & 1;
    reg = (reg << 1) | temp;
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::rla(void)
{
    u8 temp = a >> 7;
    a = (a << 1) | cf;
    zf = false;
    cf = temp & 1;
    nf = false;
    hf = false;
}

void cCpu::rlhl(void)
{
    WARNING(hl() < 0x4000, "rlhl??");
    u8 temp = mMemory->readByte(hl());
    rl(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::rrc(u8 &reg)
{
    cf = reg & 1;
    reg = (reg >> 1) | (cf << 7);
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::rrca(void)
{
    cf = a & 1;
    a = (a >> 1) | (cf << 7);
    zf = false;
    nf = false;
    hf = false;
}

void cCpu::rrchl(void)
{
    WARNING(hl() < 0x4000, "rrchl??");
    u8 temp = mMemory->readByte(hl());
    rrc(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::rr(u8 &reg)
{
    bool temp = reg & 1; //Keep the first bit
    reg = (reg >> 1) | (cf << 7); //Shift n right and put carry at the top
    cf = temp; //Put the kept n bit into carry
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::rra(void)
{
    bool temp = a & 1; //Keep the first bit
    a = (a >> 1) | (cf << 7); //Shift n right and put carry at the top
    cf = temp; //Put the kept n bit into carry
    zf = false;
    nf = false;
    hf = false;
}

void cCpu::rrhl(void)
{
    WARNING(hl() < 0x4000, "rrhl??");
    u8 temp = mMemory->readByte(hl());
    rr(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::rst(u8 val)
{
    call(true, val);
}

void cCpu::sbc(u8 val)
{
    u16 temp = a - val - cf;
    nf = true;
    //hf = (temp & 0xF) > (a & 0xF);
    cf = (temp > a);
    hf = ((a ^ val ^ (temp & 0xFF)) & 0x10 ? true : false);

    a = temp & 0xFF;
    zf = (a == 0);
}

void cCpu::set(u8 bit, u8 &reg)
{
    reg |= (1 << (bit >> 3));
}

void cCpu::sethl(u8 bit)
{
    WARNING(hl() < 0x4000, "sethl??");
    u8 temp = mMemory->readByte(hl());
    set(bit, temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::sla(u8 &reg)
{
    cf = (reg >> 7) & 1;
    reg <<= 1;
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::slahl(void)
{
    WARNING(hl() < 0x4000, "slahl??");
    u8 temp = mMemory->readByte(hl());
    sla(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::sra(u8 &reg)
{
    cf = reg & 1;
    reg = (reg >> 1) | (reg & 0x80);
    zf = (reg == 0);
    hf = false;
    nf = false;
}

void cCpu::srahl(void)
{
    WARNING(hl() < 0x4000, "srahl??");
    u8 temp = mMemory->readByte(hl());
    sra(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::srl(u8 &reg)
{
    cf = reg & 1;
    reg >>= 1;
    zf = (reg == 0);
    nf = false;
    hf = false;
}

void cCpu::srlhl(void)
{
    WARNING(hl() < 0x4000, "srlhl??");
    u8 temp = mMemory->readByte(hl());
    srl(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::sub(u8 val)
{
    zf = (a == val);
    nf = true;
    hf = (val & 0xF) > (a & 0xF);
    cf = (val > a);
    a -= val;
}

void cCpu::swap(u8 &reg)
{
    reg = ((reg & 0xF) << 4) | (reg >> 4);
    zf = (reg == 0);
    nf = false;
    hf = false;
    cf = false;
}

void cCpu::swaphl(void)
{
    WARNING(hl() < 0x4000, "swaphl??");
    u8 temp = mMemory->readByte(hl());
    swap(temp);
    mMemory->writeByte(hl(), temp);
}

void cCpu::z8xor(u8 val)
{
    a ^= val;
    zf = (a == 0);
    nf = false;
    hf = false;
    cf = false;
}

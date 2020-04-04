#include "cCpu.h"

#include <iostream>

#ifdef USE_SDL_NET
cNet net;
#endif

cCpu::cCpu() : mCyclesSum{0} {
    mMemory = nullptr;
    mOpcodeCyclesTable = {
        1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 0, 3, 2, 2, 1, 1, 2, 1,
        2, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
        2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
        1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
        1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2,
        1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
        1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
        1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
        2, 3, 3, 3, 3, 4, 2, 4, 2, 4, 3, 0, 3, 3, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4,
        2, 4, 3, 0, 3, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
        3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4};
    for (int &i : mOpcodeCyclesTable) {
        i *= 4;
    }

    mCBOpcodeCyclesTable = {
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2,
        2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
        2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
        2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2};
    for (int &i : mCBOpcodeCyclesTable) {
        i *= 4;
    }
}

cCpu::~cCpu() {
    if (mMemory) delete mMemory;
}

u8 cCpu::flags() {
    return (u8)(z_flag << 7u) | (u8)(n_flag << 6u) | (u8)(h_flag << 5u) |
           (u8)(c_flag << 4u);
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

void cCpu::saveState(int number) {}

void cCpu::loadState(int number) {}

bool cCpu::init_cpu(std::string file_name) {
    af(0x11B0);
    bc(0x0013);
    de(0x00D8);
    hl(0x014D);
    pc = 0x0100;
    sp = 0xFFFE;

    std::cout << "Rom....";
    mMemory = new MemoryMap;
    if (mMemory == nullptr) {
        std::cout << "Failure Type A" << std::endl;
        return false;
    }
    if (!mMemory->load_rom(file_name)) {
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

    std::cout << "Everything Done!!" << std::endl;
    return true;
}

int cCpu::checkInterrupts() {
    auto interrupt = mMemory->getEnabledInterrupts();
    if (interruptsEnabled && interrupt > 0) {
        interruptsEnabled = false;
        if (interrupt & eInterrupts::VBLANK) {  // v-blank
            call(true, 0x0040);
            mMemory->resetInterruptRequest(eInterrupts::VBLANK);
        } else if (interrupt & eInterrupts::LCDC) {  // LCDC
            call(true, 0x0048);
            mMemory->resetInterruptRequest(eInterrupts::LCDC);
        } else if (interrupt & eInterrupts::TIMER) {  // timer
            call(true, 0x0050);
            mMemory->resetInterruptRequest(eInterrupts::TIMER);
        } else if (interrupt & eInterrupts::SERIAL) {  // Serial Transfer
            call(true, 0x0058);
            mMemory->resetInterruptRequest(eInterrupts::SERIAL);
        } else if (interrupt & eInterrupts::JOYPAD) {  // P10-P13
            call(true, 0x0060);
            mMemory->resetInterruptRequest(eInterrupts::JOYPAD);
        }
        return 20;
    }
    return 0;
}

void cCpu::initRTCTimer() {
    struct tm currentTime;
    time_t timer;

    timer = time(NULL);
    currentTime = *localtime(&timer);
    // printf("%d:%d:%d\n", currentTime.tm_hour, currentTime.tm_min,
    // currentTime.tm_sec);

    mMemory->rtc.sec = currentTime.tm_sec;
    mMemory->rtc.min = currentTime.tm_min;
    mMemory->rtc.hr = currentTime.tm_hour - 10;
    mMemory->rtc.dl = currentTime.tm_wday;

    mMemory->rtc2.sec = currentTime.tm_sec;
    mMemory->rtc2.min = currentTime.tm_min;
    mMemory->rtc2.hr = currentTime.tm_hour - 10;
    mMemory->rtc2.dl = currentTime.tm_wday;
}


int cCpu::fetchOpCode() {
    return mMemory->readByte(pc++);
}


void cCpu::runFrame() {
    for (int line = 0; line < 154; ++line) {
        if (line == 144) {
            fullUpdate();
            mMemory->mDisplay->updateScreen();
        }
        runScanLine();
    }
}

void cCpu::runScanLine() {
    do {
        u8 opCode = fetchOpCode();
        auto cycles = 0;
        updateIMEFlag();
        if (opCode == 0xCB) {
            u8 cbOpCode = fetchOpCode();
            executeCBOpCode(cbOpCode);
            cycles = mCBOpcodeCyclesTable[cbOpCode];
        } else {
            executeOpCode(opCode);
            cycles = mOpcodeCyclesTable[opCode];
        }
        cycles += checkInterrupts();
        mMemory->updateIO(cycles);
        rtcCount += cycles;
    } while (!mMemory->mDisplay->hasLineFinished());
    mMemory->HBlankHDMA();
}

void cCpu::fullUpdate() {
    mMemory->mInput->update();
    if (mMemory->mInput->isKeyPressed(GBK_ESCAPE)) {
        mMemory->save_sram();
        mMemory->mSound->turnOff();
        isRunning = false;
    }
    if (mMemory->mInput->isKeyPressed(GBK_s)) {
        saveState(0);
    }
    if (mMemory->mInput->isKeyPressed(GBK_l)) {
        loadState(0);
    }

    if (mMemory->mInput->isKeyPressed(GBK_KP_PLUS))
        if (fpsSpeed < 5) fpsSpeed++;

    if (mMemory->mInput->isKeyPressed(GBK_KP_MINUS))
        if (fpsSpeed > 1) fpsSpeed--;

    if (!mMemory->mInput->isKeyPressed(GBK_SPACE)) {
        auto temp = SDL_GetTicks();
        if (time1 > temp) {
            SDL_Delay(time1 - temp + 10);
        }
        time1 = temp + ((17 / fpsSpeed));
    }
}

void cCpu::updateIMEFlag() {
    if (intStatus == 3) {
        // Turn on interrupts
        interruptsEnabled = true;
        intStatus = 0;
    }
    if (intStatus == 4) {
        // Turn off interrupts
        interruptsEnabled = false;
        intStatus = 0;
    }
    if (intStatus == 1)  // Turn on in the next opcode
        intStatus = 3;
    if (intStatus == 2)  // Turn off in the next opcode
        intStatus = 4;
}

#include "cpu.h"
#include <print>

namespace {
    constexpr std::array<int, 256> kOpBase = {
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
        3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4
    };

    constexpr std::array<int, 256> kCbOpBase = {
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
        2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2
    };

    consteval std::array<int, 256> times4(std::array<int, 256> a) {
        for (auto &v: a) v *= 4;
        return a;
    }

    constinit std::array<int, 256> kOpCycles = times4(kOpBase);
    constinit std::array<int, 256> kCbOpCycles = times4(kCbOpBase);
}

cCpu::cCpu() noexcept : mOpcodeCyclesTable{kOpCycles}, mCBOpcodeCyclesTable{kCbOpCycles} {
}

u16 cCpu::af() {
    return static_cast<u16>((a << 8u) | f.byte);
}

void cCpu::af(const u16 value) {
    a = value >> 8u;
    f.byte = value & 0xF0u;
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

[[nodiscard]] std::expected<void, std::string> cCpu::init_cpu(std::string_view file_name) {
    af(0x11B0);
    bc(0x0013);
    de(0x00D8);
    hl(0x014D);
    pc = 0x0100;
    sp = 0xFFFE;

    std::println("Allocating resources");
    try {
        mMemory = std::make_unique<MemoryMap>();
    } catch (const std::bad_alloc&) {
        return std::unexpected("Memory allocation failed for MemoryMap");
    }

    std::println("Loading Rom into memory");
    const std::string path(file_name);
    if (!mMemory->load_rom(path)) {
        return std::unexpected(std::format("Failed to load ROM {}", file_name));
    }
    std::println("Rom loaded successfully");
    std::println("Initializing CPU");

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

#if SDL_VERSION_ATLEAST(2, 0, 18)
    const auto now = SDL_GetTicks64();
    time1 = static_cast<u32>(now + 16);
    time2 = static_cast<u32>(now + 1000);
#else
    time1 = SDL_GetTicks() + 16u;
    time2 = SDL_GetTicks() + 1000u;
#endif

    isRunning = true;
    fpsSpeed = 1;

    std::println("Starting emulation! Have fun!");
    return {};
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
        if (line < 144) {
            mMemory->HBlankHDMA();
        } else if (line == 144) {
            fullUpdate();
            // mMemory->mDisplay.updateScreen();
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
}

void cCpu::fullUpdate() {
    mMemory->mInput->update();
    if (mMemory->mInput->isKeyPressed(GBK_ESCAPE)) {
        mMemory->save_sram();
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
            SDL_Delay((time1 - temp) * 2);
        } else {
            time1 = temp + ((17 / fpsSpeed));
        }
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

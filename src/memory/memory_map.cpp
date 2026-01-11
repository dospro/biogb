#include "memory_map.h"

#include <expected>
#include <fstream>
#include <print>


std::expected<void, std::string> MemoryMap::load_rom(const std::string_view file_name) {
    mRomFilename = std::string(file_name);
    try {
        RomLoader loader{file_name};
        mRom = loader.get_rom();
        mIsColor = loader.is_color();
        mbc_type = loader.get_mbc_type();
        with_timer = loader.has_timer();
        init_ram(loader.get_ram_banks());
    } catch (std::exception &e) {
        return std::unexpected(e.what());
    }
    init_wram(mIsColor);
    load_sram();
    if (const auto result = init_sub_systems(); !result) [[unlikely]] {
        return std::unexpected(result.error());
    }
    sgb.add_packet_listener([this](const u8 command, const std::vector<SGB::Packet> &packets) {
        handle_sgb_command(command, packets);
    });
    return {};
}

void MemoryMap::init_ram(const int ram_banks) {
    for (int i = 0; i < ram_banks; ++i) {
        mRam.emplace_back(std::array<u8, 0x2000>{});
    }
}

void MemoryMap::init_wram(const bool is_color) {
    mWRam.push_back(std::array<u8, 0x1000>{});
    mWRam.push_back(std::array<u8, 0x1000>{});
    if (is_color) {
        for (int i = 0; i < 6; ++i) {
            mWRam.push_back(std::array<u8, 0x1000>{});
        }
    }
}

std::expected<void, std::string> MemoryMap::init_sub_systems() noexcept {
    std::print("Starting Display -> ");
    try {
        mDisplay = std::make_unique<cDisplay>(mIsColor);
    } catch (const std::exception &) {
        std::println("Failed");
        return std::unexpected("Failed to start display system");
    }
    std::println("Ok");

    std::print("Starting Sound System -> ");
    try {
        mSound = std::make_unique<cSound>(44100);
    } catch (const std::exception &) {
        std::println("Failed");
        return std::unexpected("Failed to start sound system");
    }
    std::println("Ok");

    std::print("Starting Input System -> ");
    try {
        mInput = std::make_unique<cInput>();
    } catch (const std::exception &) {
        std::println("Failed");
        return std::unexpected("Failed to start input system");
    }
    std::println("Ok");
    try {
        mTimer = std::make_unique<cTimer>();
    } catch (const std::exception &) {
        return std::unexpected("Failed to start timer system");
    }
    return {};
}

u8 MemoryMap::readByte(const u16 address) {
    if (address < 0x8000) [[likely]] return readRom(address);
    else if (address < 0xA000)
        return mDisplay->readFromDisplay(address);
    else if (address < 0xC000)
        return readRam(address);
    else if (address < 0xD000)
        return mWRam[0][address - 0xC000];
    else if (address < 0xE000)
        return mWRam[wRamBank][address - 0xD000];
    else if (address < 0xFE00)
        return readByte(address - 0x2000);
    else if (address < 0xFEA0)
        return mDisplay->readFromDisplay(address);
    else if (address < 0xFF00)
        ;
    else if (address < 0xFF01)
        return mInput->readRegister();
    else if (address < 0xFF10)
        return readIO(address);
    else if (address < 0xFF40)
        return mSound->readFromSound(address);
    else if (address < 0xFF80)
        return readIO(address);
    else if (address < 0xFFFE)
        return mHRam[address - 0xFF80];
    else
        return readIO(address);

    return 0xFF;
}

int MemoryMap::readIO(const int a_address) {
    switch (a_address) {
        case 0xFF00:
            if (sgb.mlt_is_active()) {
                return sgb.mlt_get_current_player();
            }
            return IOMap[0];
        case 0xFF04:
        case 0xFF05:
        case 0xFF06:
        case 0xFF07: return mTimer->readRegister(a_address);
        case 0xFF0F: return readIFRegister();
        case 0xFF4D: return (static_cast<int>(mPrepareSpeedChange)) | (mCurrentSpeed << 7);
        case 0xFF40:  // LCDC
        case 0xFF41:  // STAT
        case 0xFF42:  // SCY
        case 0xFF43:  // SCX
        case 0xFF44:  // LY
        case 0xFF45:  // LYC
        case 0xFF47:  // BGP
        case 0xFF48:  // OBP0
        case 0xFF49:  // OBP1
        case 0xFF4A:  // WY
        case 0xFF4B:  // WX
        case 0xFF4F:  // VRAM Bank
        case 0xFF68:  // BGPI
        case 0xFF69:  // BGPD
        case 0xFF6A:
        case 0xFF6B: return mDisplay->readFromDisplay(a_address);
        case 0xFF51: return hdma.src & 0xFF;
        case 0xFF52: return (hdma.src >> 8) & 0xFF;
        case 0xFF53: return hdma.dest & 0xFF;
        case 0xFF54: return (hdma.dest >> 8) & 0xFF;
        case 0xFF55: return (hdma.mode << 7) | (hdma.length / 0x10 - 1);
        case 0xFFFF: return IERegister;
        default: return IOMap[a_address & 0xFF];
    }
}

u8 MemoryMap::readRom(u16 a_address) const noexcept {
    if (a_address < 0x4000) return mRom[0][a_address];
    else
        return mRom[romBank][a_address - 0x4000];
}

u8 MemoryMap::readRam(const u16 address) const {
    if (rtc.areRtcRegsSelected) {
        switch (rtc.rtcRegSelect) {
            case 0x8: return rtc.sec;
            case 0x9: return rtc.min;
            case 0xA: return rtc.hr;
            case 0xB: return rtc.dl;
            case 0xC: return rtc.dh;
            default: return mRam[ramBank][address - 0xA000];
        }
    }
    if (mRam.empty()) {
        return 0xFF;
    }
    return mRam[ramBank][address - 0xA000];
}

void MemoryMap::send_command(u16 address, u8 value) {
    if (mbc_type == MBCTypes::MBC1) {
        sendMBC1Command(address, value);
    } else if (mbc_type == MBCTypes::MBC2) {
        sendMBC2Command(address, value);
    } else if (mbc_type == MBCTypes::MBC3) {
        sendMBC3Command(address, value);
    } else if (mbc_type == MBCTypes::MBC5) {
        sendMBC5Command(address, value);
    }
}

void MemoryMap::writeByte(u16 a_address, u8 a_value) {
    if (a_address < 0x8000) {
        send_command(a_address, a_value);
    } else if (a_address < 0xA000)
        mDisplay->writeToDisplay(a_address, a_value);
    else if (a_address < 0xC000) {
        if (rtc.areRtcRegsSelected) {
            writeRTCRegister(a_value);
        } else if (!mRam.empty()) {
            mRam[ramBank][a_address - 0xA000] = a_value;
        }
    } else if (a_address < 0xD000)
        mWRam[0][a_address - 0xC000] = a_value;
    else if (a_address < 0xE000)
        mWRam[wRamBank][a_address - 0xD000] = a_value;
    else if (a_address < 0xFE00)
        writeByte(a_address - 0x2000, a_value);
    else if (a_address < 0xFEA0)
        mDisplay->writeToDisplay(a_address, a_value);
    else if (a_address < 0xFF00)
        ;
    else if (a_address < 0xFF80)
        writeIO(a_address, a_value);
    else if (a_address < 0xFFFE)
        mHRam[a_address - 0xFF80] = a_value;
    else
        writeIO(a_address, a_value);
}

void MemoryMap::sendMBC1Command(const u16 a_address, const u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) {
        if (mRomMode) {
            romBank = a_value & 0x1F;
            if (romBank == 0) romBank++;
        } else {
            romBank = (romBank & 0x60) | (a_value & 0x1F);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60) romBank++;
        }
    } else if (a_address >= 0x4000 && a_address < 0x6000) {
        if (mRomMode) {
            romBank = (romBank & 0x1F) | ((a_value & 3) << 5);
            if (romBank == 0 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60) romBank++;
        } else {
            ramBank = a_value & 3;
        }
    } else if (a_address >= 0x6000 && a_address < 0x8000)
        mRomMode = (a_value & 1) == 0;
}

void MemoryMap::sendMBC2Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) {
        // rom bank change
        romBank = a_value & 0xF;
        if (romBank == 0) romBank++;
    }
}

void MemoryMap::sendMBC3Command(const u16 a_address, const u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x4000) romBank = a_value & 0x7F;
    else if (a_address >= 0x4000 && a_address < 0x6000) {
        switch (a_value) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
                ramBank = a_value;
                rtc.areRtcRegsSelected = false;
                break;
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
                rtc.rtcRegSelect = a_value;
                rtc.areRtcRegsSelected = true;
                break;
            default:
                std::println("Send MBC3 command: {:x}", a_value);
                break;
        }
    } else if (a_address >= 0x6000 && a_address < 0x8000) {
        const u8 prev_latch = rtc.latch;
        rtc.latch = a_value;

        if (prev_latch == 0 && a_value == 1) {
            rtc.dh = rtc2.dh;
            rtc.dl = rtc2.dl;
            rtc.hr = rtc2.hr;
            rtc.min = rtc2.min;
            rtc.sec = rtc2.sec;
        }
    }
}

void MemoryMap::sendMBC5Command(u16 a_address, u8 a_value) {
    if (a_address >= 0x2000 && a_address < 0x3000)  // 8 lower bits rom bank change
    {
        MBC5LowAddress = a_value;
        romBank = ((MBC5HighAddress << 8) | MBC5LowAddress) & 0x1FF;
    } else if (a_address >= 0x3000 && a_address < 0x4000)  // 9 bit of rom bank change
    {
        MBC5HighAddress = a_value & 1;
        romBank = ((MBC5HighAddress << 8) | MBC5LowAddress) & 0x1FF;
    } else if (a_address >= 0x4000 && a_address < 0x6000)  // ram bank change
        ramBank = a_value & 0xF;
}

void MemoryMap::writeRTCRegister(u8 a_value) {
    switch (rtc.rtcRegSelect) {
        case 0x8:
            rtc2.sec = a_value;
            rtc.sec = a_value;
            break;
        case 0x9:
            rtc2.min = a_value;
            rtc.min = a_value;
            break;
        case 0xA:
            rtc2.hr = a_value;
            rtc.hr = a_value;
            break;
        case 0xB:
            rtc2.dl = a_value;
            rtc.dl = a_value;
            break;
        case 0xC:
            rtc2.dh = a_value;
            rtc.dh = a_value;
            break;
        default:
            std::println("Sending rtc command to address", rtc.rtcRegSelect);
    }
}

void MemoryMap::writeIO(const u16 a_address, const u8 a_value) {
    if (a_address >= 0xFF10 && a_address < 0xFF40) {
        mSound->writeToSound(a_address, a_value);
        return;
    }
    switch (a_address) {
        case 0xFF00: // P1-Controls
            sgb.mlt_change_joyp(false);
            if (sgb.is_sgb_transfer_mode()) {
                if ((a_value & 0x30) == 0x10) {
                    sgb.send_bit(1);
                } else if ((a_value & 0x30) == 0x20) {
                    sgb.send_bit(0);
                }
            } else if ((a_value & 0x30) == 0) {
                // SGB Reset
                sgb.start_transfer_mode();
            } else if ((a_value & 0x30) == 0x30) {
                sgb.mlt_change_joyp(true);
            } else {
                mInput->writeRegister(a_value);
            }
            break;
        case 0xFF01:  // SB-Serial Transfer data
            ST.trans = a_value;
            IOMap[a_address & 0xFF] = a_value;
            break;
        case 0xFF02:  // SC-SIO Control
            ST.start = (a_value >> 7) & 1;
            ST.speed = (a_value >> 1) & 1;
            ST.cType = (a_value & 1);
            IOMap[a_address & 0xFF] = a_value;
            if (ST.start) {
                if (ST.cType) {
                    IOMap[0xFF01 & 0xFF] = 0xFF;
                    IOMap[a_address & 0xFF] = a_value & 0x7F;
                    IOMap[0xFF0F & 0xFF] |= 8;
                }
            }
            break;
        case 0xFF04:  // DIV-Divider Register
        case 0xFF05:  // TIMA Register
        case 0xFF06:  // TMA Register
        case 0xFF07:  // TAC Register
            mTimer->writeRegister(a_address, a_value);
            break;
        case 0xFF0F: writeIFRegister(a_value); break;  // IF Register
        case 0xFF46: DMATransfer(a_value); break;  // DMA
        case 0xFF40:  // LCDC
        case 0xFF41:  // STAT
        case 0xFF42:  // SCY
        case 0xFF43:  // SCX
        case 0xFF44:  // LY
        case 0xFF45:  // LYC
        case 0xFF47:  // BGP
        case 0xFF48:  // OBP0
        case 0xFF49:  // OBP1
        case 0xFF4A:  // WY
        case 0xFF4B:  // WX
        case 0xFF4F:  // VRAM bank
            mDisplay->writeToDisplay(a_address, a_value);
            break;
        case 0xFF4D: mPrepareSpeedChange = (a_value & 1) == 1; break;
        case 0xFF51: hdma.src = (hdma.src & 0xF0) | (a_value << 8); break;             // HDMA Source High
        case 0xFF52: hdma.src = (hdma.src & 0xFF00) | (a_value & 0xF0); break;         // HDMA Source Low
        case 0xFF53: hdma.dest = (hdma.dest & 0xFF) | ((a_value & 0x1F) << 8); break;  // HDMA Dest High
        case 0xFF54: hdma.dest = (hdma.dest & 0x1F00) | (a_value & 0xF0); break;       // HDMA Dest Low
        case 0xFF55:  // HDMA Transfer
            hdma.mode = ((a_value >> 7) & 1) != 0;
            hdma.length = ((a_value & 0x7F) + 1) * 0x10;
            if (hdma.active && !hdma.mode) {  //  Stop a current H-BLANK HDMA
                hdma.active = false;
                hdma.mode = false;
            } else if (!hdma.mode && !hdma.active) {  // Start a General HDMA
                HDMATransfer(hdma.src, hdma.dest, hdma.length);
                hdma.mode = true;
                hdma.length = 0x800;
            } else if(hdma.mode) {  // Start H-Blank HDMA
                hdma.active = true;
                hdma.mode = false;
            }
            break;
        case 0xFF68:
        case 0xFF69:
        case 0xFF6A:
        case 0xFF6B: mDisplay->writeToDisplay(a_address, a_value); break;
        case 0xFF70:
            IOMap[a_address & 0xFF] = a_value;
            if (mIsColor) {
                wRamBank = a_value & 7;
                if (wRamBank == 0) wRamBank++;
            }
            break;
        case 0xFFFF: IERegister = a_value; break;
        default: IOMap[a_address & 0xFF] = a_value;
    }
}

void MemoryMap::DMATransfer(u8 address) {
    for (int src = address << 8, dest = 0xFE00; dest < 0xFEA0; dest++, src++) {
        mDisplay->writeToDisplay(dest, readByte(src));
    }
}

void MemoryMap::HDMATransfer(u16 source, u16 dest, u32 length) {
    for (int i = 0; i < length; i++) {
        writeByte(0x8000 + dest + i, readByte(source + i));
    }
}

void MemoryMap::HBlankHDMA() {
    if (hdma.active) {
        for (int i = 0; i < 0x10; ++i) {
            writeByte(0x8000 + hdma.dest + i, readByte(hdma.src + i));
        }
        hdma.dest += 0x10;
        hdma.src += 0x10;
        hdma.length -= 0x10;

        if (hdma.length <= 0) {
            hdma.mode = true;
            hdma.length = 0x800;
            hdma.active = false;
        }
    }
}

void MemoryMap::rtcCounter() {
    if (((rtc2.dh >> 6) & 1) != 0) {
        return;
    }
    rtc2.sec++;
    if (rtc2.sec < 60) {
        return;
    }
    rtc2.sec = 0;
    rtc2.min++;
    if (rtc2.min < 60) {
        return;
    }
    rtc2.min = 0;
    rtc2.hr++;
    if (rtc2.hr < 24) {
        return;
    }
    rtc2.hr = 0;
    if (rtc2.dl < 0xFF) rtc2.dl++;
    else {
        if ((rtc2.dh & 1) == 0) rtc2.dh |= 1;
        else {
            rtc2.dh |= 0x80;
            rtc2.dh &= 0xFE;
            rtc2.dl = 0;
        }
    }
}

void MemoryMap::save_sram() {
    const std::filesystem::path rom_path(mRomFilename);
    const std::filesystem::path save_dir{"savs/"};
    const auto base_name = rom_path.stem();

    std::println("Saving: {}", base_name.string());

    // Ensure save directory exists
    std::error_code ec;
    std::filesystem::create_directories(save_dir, ec);
    if (ec) {
        std::println(stderr, "WARNING: Failed to create save directory: {}", ec.message());
        return;
    }

    const auto sav_filename = save_dir / (base_name.string() + ".sav");
    if (auto save_file = std::ofstream{sav_filename, std::ios::binary}) {
        for (const auto &ram_bank: mRam)
            save_file.write(reinterpret_cast<const char *>(ram_bank.data()), ram_bank.size());
    } else {
        std::println(stderr, "WARNING: Failed to create sav file");
        return;
    }

    if (with_timer) {
        const auto rtc_path = save_dir / (base_name.string() + ".rtc");
        if (auto rtc_file = std::ofstream{rtc_path, std::ios::binary}) {
            rtc_file.write(reinterpret_cast<const char *>(&rtc), sizeof(RTC_Regs));
            rtc_file.write(reinterpret_cast<const char *>(&rtc2), sizeof(RTC_Regs));
        } else {
            std::println(stderr, "WARNING: Failed to create RTC file");
            return;
        }
    }
    std::println("Successfully saved: {}", base_name.string());
}

void MemoryMap::load_sram() {
    const std::filesystem::path rom_path(mRomFilename);
    const std::filesystem::path save_dir{"savs/"};
    const auto base_name = rom_path.stem();

    std::println("Loading: {}", base_name.string());

    const auto sav_filename = save_dir / (base_name.string() + ".sav");

    if (auto save_file = std::ifstream{sav_filename, std::ios::binary}) {
        for (auto &ram_bank: mRam)
            save_file.read(reinterpret_cast<char *>(ram_bank.data()), ram_bank.size());
    } else {
        std::println(stderr, "WARNING: Failed to read sav file");
        return;
    }

    if (with_timer) {
        const auto rtc_path = save_dir / (base_name.string() + ".rtc");
        if (auto rtc_file = std::ifstream{rtc_path, std::ios::binary}) {
            rtc_file.read(reinterpret_cast<char *>(&rtc), sizeof(RTC_Regs));
            rtc_file.read(reinterpret_cast<char *>(&rtc2), sizeof(RTC_Regs));
        } else {
            std::println(stderr, "WARNING: Failed to load RTC file");
            return;
        }
    }
}

void MemoryMap::updateIO(const int a_cycles) {
    mDisplay->update(a_cycles >> mCurrentSpeed);
    mSound->updateCycles(a_cycles >> mCurrentSpeed);
    mTimer->update(a_cycles);
}

int MemoryMap::changeSpeed() {
    if (!mIsColor) {
        return 0;
    }
    if (mPrepareSpeedChange) {
        if (mCurrentSpeed == 0) mCurrentSpeed = 1;
        else
            mCurrentSpeed = 0;
        mPrepareSpeedChange = false;
    }
    return mCurrentSpeed;
}

u8 MemoryMap::getEnabledInterrupts() {
    return IERegister & readIFRegister();
}

void MemoryMap::resetInterruptRequest(int interrupt) {
    switch (interrupt) {
        case eInterrupts::VBLANK: mDisplay->mVBlankInterruptRequest = false; break;
        case eInterrupts::LCDC: mDisplay->mLCDInterruptRequest = false; break;
        case eInterrupts::TIMER: mTimer->InterruptBit = false; break;
        default: std::println("Resetting other interrupt {}", interrupt); break;
    }
}

int MemoryMap::readIFRegister() {
    u8 vblank = mDisplay->mVBlankInterruptRequest ? eInterrupts::VBLANK : 0;
    u8 lcdc = mDisplay->mLCDInterruptRequest ? eInterrupts::LCDC : 0;
    u8 timer = mTimer->InterruptBit ? eInterrupts::TIMER : 0;
    u8 serial = 0;
    u8 joypad = 0;
    return serial | joypad | timer | vblank | lcdc;
}

void MemoryMap::writeIFRegister(u8 value) {
    mDisplay->mVBlankInterruptRequest = (value & eInterrupts::VBLANK) != 0;
    mDisplay->mLCDInterruptRequest = (value & eInterrupts::LCDC) != 0;
    mTimer->InterruptBit = (value & eInterrupts::TIMER) != 0;
    if ((value & eInterrupts::SERIAL) || (value & eInterrupts::JOYPAD)) {
        std::println("Other interrupts request");
    }
}

void MemoryMap::handle_sgb_command(const u8 command, const std::vector<SGB::Packet> &packets) {
    switch (command) {
        case PAL_TRN:
            for (size_t i = 0; i< 0x1000; ++i) {
                const auto data = readByte(0x8000 + i * 2) | readByte(0x8000 + i * 2 + 1) << 8;
                sgb.write_sgb_palette(i, data);
            }
            break;
        case MASK_EN: {
            const auto mask = packets[0][1];
            if (mask == 0) {
                mDisplay->toggle_freeze_screen(true);
            } else if (mask == 1 || mask == 2 || mask == 3) {
                mDisplay->toggle_freeze_screen(false);
            }
        }
        break;
        default:
            break;
    }
}

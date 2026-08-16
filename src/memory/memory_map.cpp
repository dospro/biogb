#include "memory_map.h"

#include <expected>
#include <filesystem>
#include <fstream>
#include <print>


std::expected<void, std::string> MemoryMap::load_rom(const std::string_view file_name) {
    mRomFilename = std::string(file_name);
    try {
        RomLoader loader{file_name};
        is_battery_backed = loader.has_battery();
        has_rtc = loader.has_timer();
        cartridge = loader.get_cartridge_interface();
        model = selected_console_model(loader.get_support());
        mIsColor = model == ConsoleModel::CGB;
    } catch (std::exception &e) {
        return std::unexpected(e.what());
    }
    init_wram(mIsColor);
    load_sram();
    if (const auto result = init_sub_systems(); !result) [[unlikely]] {
        return std::unexpected(result.error());
    }
    return {};
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
        return readIO(address);
    else if (address < 0xFF10)
        return readIO(address);
    else if (address < 0xFF40)
        return mSound->readFromSound(address);
    else if (address < 0xFF80)
        return readIO(address);
    else if (address < 0xFFFF)
        return mHRam[address - 0xFF80];
    else
        return readIO(address);

    return 0xFF;
}

int MemoryMap::readIO(const int a_address) {
    switch (a_address) {
        case 0xFF00: {
            const int value = mInput->readRegister();
            /*
             * P15 (bit 5) and P14 (bit 4) are the two button-group selects and are active low,
             * so 0x30 is the one state where the ROM has deselected both groups. That matters
             * because the low nibble is shared: while a group is selected it carries that
             * group's button state and must be left alone. Only with both deselected is it
             * free for the SGB to drive with the active player's ID.
             *
             * That is also precisely why this doubles as the SGB detection signal -- on
             * unenhanced hardware nothing drives those lines, so they float high and the ROM
             * reads 0xF here forever no matter how many times it polls.
             *
             * & 0xF0 replaces only the low nibble: bits 5-4 keep whatever the ROM last wrote
             * to them, and bits 7-6 are unused and read high.
             */
            if (is_sgb() && (value & 0x30) == 0x30 && sgb.mlt_is_multiplayer()) {
                return (value & 0xF0) | sgb.mlt_id_nibble();
            }
            return value;
        }
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

u8 MemoryMap::readRom(const u16 a_address) const noexcept {
    return std::visit([a_address](const auto &mbc) { return mbc.read_rom(a_address); }, cartridge);
}

u8 MemoryMap::readRam(const u16 address) const {
    return std::visit([address](const auto &mbc) { return mbc.read_ram(address); }, cartridge);
}

void MemoryMap::writeByte(const u16 a_address, const u8 a_value) {
    if (a_address < 0x8000) {
        std::visit([a_address, a_value](auto &mbc) { mbc.write(a_address, a_value); }, cartridge);
    } else if (a_address < 0xA000) {
        mDisplay->writeToDisplay(a_address, a_value);
    } else if (a_address < 0xC000) {
        std::visit([a_address, a_value](auto &mbc) { mbc.write_ram(a_address, a_value); }, cartridge);
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
    else if (a_address < 0xFFFF)
        mHRam[a_address - 0xFF80] = a_value;
    else
        writeIO(a_address, a_value);
}


void MemoryMap::writeIO(const u16 a_address, const u8 a_value) {
    if (a_address >= 0xFF10 && a_address < 0xFF40) {
        mSound->writeToSound(a_address, a_value);
        return;
    }
    switch (a_address) {
        case 0xFF00: // P1-Controls
            if (is_sgb()) sgb.write(a_value);
            mInput->writeRegister(a_value);
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

void MemoryMap::save_sram() {
    if (!is_battery_backed) {
        return;
    }
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
        std::visit([&save_file](const auto &mbc) { mbc.save(save_file); }, cartridge);
    } else {
        std::println(stderr, "WARNING: Failed to create sav file");
        return;
    }

    if (has_rtc) {
        if (const auto *mbc3 = std::get_if<MBC3>(&cartridge)) {
            const auto rtc_path = save_dir / (base_name.string() + ".rtc");
            if (auto rtc_file = std::ofstream{rtc_path, std::ios::binary}) {
                mbc3->save_rtc(rtc_file);
            } else {
                std::println(stderr, "WARNING: Failed to create RTC file");
                return;
            }
        }
    }
    std::println("Successfully saved: {}", base_name.string());
}

void MemoryMap::load_sram() {
    if (!is_battery_backed) {
        return;
    }
    const std::filesystem::path rom_path(mRomFilename);
    const std::filesystem::path save_dir{"savs/"};
    const auto base_name = rom_path.stem();

    std::println("Loading: {}", base_name.string());

    const auto sav_filename = save_dir / (base_name.string() + ".sav");

    if (auto save_file = std::ifstream{sav_filename, std::ios::binary}) {
        std::visit([&save_file](auto &mbc) { mbc.load(save_file); }, cartridge);
    } else {
        std::println(stderr, "No existing save found for: {}", base_name.string());
        return;
    }

    if (has_rtc) {
        if (auto *mbc3 = std::get_if<MBC3>(&cartridge)) {
            const auto rtc_path = save_dir / (base_name.string() + ".rtc");
            if (auto rtc_file = std::ifstream{rtc_path, std::ios::binary}) {
                mbc3->load_rtc(rtc_file);
            } else {
                std::println(stderr, "WARNING: Failed to load RTC file");
            }
        }
    }
}

void MemoryMap::updateIO(const int a_cycles) {
    mDisplay->update(a_cycles >> mCurrentSpeed);
    mSound->updateCycles(a_cycles >> mCurrentSpeed);
    mTimer->update(a_cycles);
    if (auto *mbc3 = std::get_if<MBC3>(&cartridge)) {
        mbc3->update_rtc(a_cycles);
    }
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

std::span<const u32> MemoryMap::get_sgb_buffer(const std::span<const u32> gb_frame) {
    // if (!is_sgb()) {
    //     return gb_frame;
    // }
    return sgb.compose_frame(gb_frame);
}

void MemoryMap::execute_sgb_vram_transfer() {
    if (!is_sgb()) return;
    sgb.run_pending_transfer(mDisplay->get_sgb_bit_patterns());
}

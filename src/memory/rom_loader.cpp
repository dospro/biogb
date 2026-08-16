#include "rom_loader.h"

#include <algorithm>
#include <print>

constexpr std::array<RomType, 29> CartridgeTypes = {
    {
        {0,    "Rom Only"},
        {1,    "Rom + MBC1"},
        {2,    "Rom + MBC1 + Ram"},
        {3,    "Rom + MBC1 + Ram + Battery"},
        {5,    "Rom + MBC2"},
        {6,    "Rom + MBC2 + Battery"},
        {8,    "Rom + Ram"},
        {9,    "Rom + Ram + Battery"},
        {0xB,  "Rom + MMMO1(Not working)"},
        {0xC,  "Rom + MMMO1 + SRam(Not working)"},
        {0xD,  "Rom + MMMO1 + SRam + Battery(Not working)"},
        {0xF,  "Rom + MBC3 + Timer + Battery"},
        {0x10, "Rom + MBC3 + Timer + Ram + Battery"},
        {0x11, "Rom + MBC3"},
        {0x12, "Rom + MBC3 + Ram"},
        {0x13, "Rom + MBC3 + Ram + Battery"},
        {0x15, "Rom + MBC4"},
        {0x16, "Rom + MBC4 + Ram"},
        {0x17, "Rom + MBC4 + Ram + Battery"},
        {0x19, "Rom + MBC5"},
        {0x1A, "Rom + MBC5 + Ram"},
        {0x1B, "Rom + MBC5 + Ram + Battery"},
        {0x1C, "Rom + MBC5 + Rumble"},
        {0x1D, "Rom + MBC5 + Rumble + SRam"},
        {0x1E, "Rom + MBC5 + Rumble + SRam + Battery"},
        {0xFC, "Pocket Cammera(Not working)"},
        {0xFD, "Bandai TAMA5(Not working)"},
        {0xFE, "Hudson HuC-3(Not working)"},
        {0xFF, "Hudson HuC-1 + Ram + Battery"},
}};

constexpr std::array<BanksInfo, 12> VALID_ROM_SIZES = {
    {
        {0,    16384 << 1, 2},
        {1,    16384 << 2, 4},
        {2,    16384 << 3, 8},
        {3,    16384 << 4, 16},
        {4,    16384 << 5, 32},
        {5,    16384 << 6, 64},
        {6,    16384 << 7, 128},
        {7,    16384 << 8, 256},
        {8,    16384 << 9, 512},
        {0x52, 16384 * 72, 72},
        {0x53, 16384 * 80, 80},
        {0x54, 16384 * 96, 96},
}};

constexpr std::array<BanksInfo, 6> VALID_RAM_SIZES = {
    {
        {0, 0,      0},
        {1, 2048,   1}, // Unused
        {2, 8192,   1},
        {3, 32768,  4},
        {4, 131072, 16},
        {5, 65536, 8},
}};

RomLoader::RomLoader(const std::string_view file_name) {
    file.open(file_name, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening the file");
    }
    read_header();
    load_rom(rom_banks);
    file.close();
}

[[nodiscard]] CartridgeSupport RomLoader::get_support() const {
    return support;
}

[[nodiscard]] bool RomLoader::has_timer() const {
    return with_timer;
}

[[nodiscard]] bool RomLoader::has_battery() const {
    return with_battery;
}

Cartridge RomLoader::get_cartridge_interface() {
    switch (mbc) {
        case MBCTypes::RomOnly: return RomOnly(std::move(rom), ram_banks);
        case MBCTypes::MBC1: return MBC1(std::move(rom), ram_banks);
        case MBCTypes::MBC2: return MBC2(std::move(rom));
        case MBCTypes::MBC3: return MBC3(std::move(rom), ram_banks);
        case MBCTypes::MBC5: return MBC5(std::move(rom), ram_banks);
        default:
            throw std::runtime_error("Unsupported cartridge type");
    }
}

void RomLoader::read_header() {
    std::array<u8, 0x150> header{};
    file.read(reinterpret_cast<char *>(header.data()), header.size());
    if ( file.gcount() < 0x150) {
        throw std::runtime_error("File has no valid header");
    }
    name = std::string(header.begin() + 0x134, header.begin() + 0x134 + 15);
    name.erase(std::ranges::find(name, '\0'), name.end());

    const u8 gbc_byte = header[0x143];
    if (gbc_byte == 0xC0) {
        support.color = ColorSupport::Required;
    } else if (gbc_byte == 0x80) {
        support.color = ColorSupport::Enhanced;
    } else {
        support.color = ColorSupport::None;
    }

    support.sgb = header[0x146] == 0x03 && header[0x14B] == 0x33;
    std::println("Hardware: {}", to_string(support.color));
    std::println("Super Game Boy: {}", support.sgb);

    const u8 mbc_id = header[0x147];

    mbc = calculate_mbc_type(mbc_id);
    with_timer = has_mbc_timer(mbc_id);
    with_battery = has_mbc_battery(mbc_id);

    rom_banks = calculate_rom_banks(header[0x148]);
    ram_banks = calculate_ram_banks(header[0x149]);
}

constexpr MBCTypes RomLoader::calculate_mbc_type(const u8 mbc_id) noexcept {
    const auto it = std::ranges::find(CartridgeTypes, mbc_id, &RomType::id);
    if (it != CartridgeTypes.end()) [[likely]] {
        std::println("Found: {}", it->name);
    } else {
        std::println("WARNING: Unknown chip found. Trying generic emulation.");
    }
    switch (mbc_id) {
        case 0:
            return MBCTypes::RomOnly;

        case 1:
        case 2:
        case 3:
            return MBCTypes::MBC1;

        case 5:
        case 6:
            return MBCTypes::MBC2;

        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return MBCTypes::MBC3;

        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            return MBCTypes::MBC5;

        default:
            return MBCTypes::Generic;
    }
}

int RomLoader::calculate_rom_banks(const u8 rom_size_id) {
    const auto it = std::ranges::find(VALID_ROM_SIZES, rom_size_id, &BanksInfo::id);
    if (it != VALID_ROM_SIZES.end()) [[likely]] {
        std::println("ROM Size: {}", it->size);
        std::println("ROM Banks: {}", it->banks);
        file.seekg(0, std::ios::beg);
        return it->banks;
    }

    file.seekg(0, std::ios::end);
    const int banks = static_cast<int>(file.tellg()) / 0x4000;
    std::println("WARNING: Unknown chip found. Trying generic emulation.");
    file.seekg(0, std::ios::beg);
    return banks;
}

int RomLoader::calculate_ram_banks(const u8 ram_size_id) {
    const auto it = std::ranges::find(VALID_RAM_SIZES, ram_size_id, &BanksInfo::id);
    if (it == VALID_RAM_SIZES.end()) [[unlikely]] {
        std::println("WARNING: Unknown ram size found.");
        return 0;
    }
    std::println("Ram size: {}", it->size);
    std::println("Ram banks: {}", it->banks);
    return it->banks;
}

constexpr bool RomLoader::has_mbc_timer(const u8 mbc_id) noexcept {
    return (mbc_id == 0x0F || mbc_id == 0x10);
}

constexpr bool RomLoader::has_mbc_battery(const u8 mbc_id) noexcept {
    switch (mbc_id) {
        case 0x03: // MBC1 + RAM + BATTERY
        case 0x06: // MBC2 + BATTERY
        case 0x09: // ROM + RAM + BATTERY
        case 0x0F: // MBC3 + TIMER + BATTERY
        case 0x10: // MBC3 + TIMER + RAM + BATTERY
        case 0x13: // MBC3 + RAM + BATTERY
        case 0x1B: // MBC5 + RAM + BATTERY
        case 0x1E: // MBC5 + RUMBLE + RAM + BATTERY
            return true;
        default:
            return false;
    }
}

void RomLoader::load_rom(const int banks) {
    rom.resize(banks);
    file.read(reinterpret_cast<char *>(rom.data()), 0x4000 * banks);
}
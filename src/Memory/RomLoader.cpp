#include <iostream>
#include "RomLoader.h"

const struct RomType CartrigeTypes[] = {
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
        {0xF,  "Rom + MBC3 + Timer + Battery(Timer not working)"},
        {0x10, "Rom + MBC3 + Timer + Ram + Battery(Timer not working)"},
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
};

const struct BanksInfo VALID_ROM_SIZES[] = {
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
};

const struct BanksInfo VALID_RAM_SIZES[] = {
        {0, 0,      0},
        {1, 2048,   1},
        {2, 8192,   1},
        {3, 32768,  4},
        {4, 131072, 16},
};

RomLoader::RomLoader(const std::string &file_name) {
    file.open(file_name, std::ios::binary);
    if (file.bad()) {
        throw std::runtime_error("Error opening the file");
    }
    read_header();
    load_rom(rom_banks);
    file.close();
}

std::vector<std::array<u8, 0x4000>> RomLoader::get_rom() {
    return rom;
}

bool RomLoader::is_color() {
    return color;
}

int RomLoader::get_ram_banks() {
    return ram_banks;
}

MBCTypes RomLoader::get_mbc_type() {
    return mbc;
}

void RomLoader::read_header() {
    file.seekg(0x134);
    char buffer[15];
    file.read(buffer, 15);
    name = std::string(buffer);

    file.seekg(0x143);
    u8 gbc_byte{};
    file.read(reinterpret_cast<char *>(&gbc_byte), 1);
    color = (gbc_byte == 0x80 || gbc_byte == 0xC0);

    file.seekg(0x147);
    u8 mbc_id{};
    file.read(reinterpret_cast<char *>(&mbc_id), 1);
    mbc = calculate_mbc_type(mbc_id);

    file.seekg(0x148);
    u8 rom_size_id{};
    file.read(reinterpret_cast<char *>(&rom_size_id), 1);

    file.seekg(0x149);
    u8 ram_size_id{};
    file.read(reinterpret_cast<char *>(&ram_size_id), 1);

    rom_banks = calculate_rom_banks(rom_size_id);
    ram_banks = calculate_ram_banks(ram_size_id);
}

MBCTypes RomLoader::calculate_mbc_type(u8 mbc_id) {
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

int RomLoader::calculate_rom_banks(u8 rom_size_id) {
    int banks{0};
    auto rom_size_info = std::find_if(
            std::begin(VALID_ROM_SIZES),
            std::end(VALID_ROM_SIZES),
            [rom_size_id](auto rom_size_item) { return rom_size_item.id == rom_size_id; }
    );
    banks = rom_size_info->banks;

    if (banks == 0) {
        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        banks = size / 0x4000;
        std::cout << "WARNING: Unknown chip found. Trying generic emulation." << std::endl;
    }
    file.seekg(0);
    return banks;
}

int RomLoader::calculate_ram_banks(u8 ram_size_id) {
    auto ram_size_info = std::find_if(
            std::begin(VALID_RAM_SIZES),
            std::end(VALID_RAM_SIZES),
            [ram_size_id](auto ram_size_item) { return ram_size_item.id == ram_size_id; }
    );
    return ram_size_info->banks;
}

void RomLoader::load_rom(int banks) {
    for (int i = 0; i < banks; ++i) {
        rom.push_back(std::array<u8, 0x4000>{});
        file.read(reinterpret_cast<char *>(&rom[i][0]), 0x4000);
    }
}
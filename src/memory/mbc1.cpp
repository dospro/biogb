#include "mbc1.h"

#include <istream>
#include <ostream>

MBC1::MBC1(RomBanks rom, const u16 ram_banks) : rom(std::move(rom)), ram(ram_banks) {}

u8 MBC1::read_rom(const u16 address) const {
    if (address < 0x4000) {
        return rom[0][address];
    }
    return rom[rom_bank][address - 0x4000];
}

u8 MBC1::read_ram(const u16 address) const {
    if (ram.empty() || !ram_enabled) {
        return 0xFF;
    }
    return ram[ram_bank][address - 0xA000];
}

void MBC1::write(const u16 address, const u8 value) {
    if (address < 0x2000) {
        ram_enabled = (value & 0x0F) == 0x0A;
    } else if (address < 0x4000) {
        bank1_register = value & 0x1F;
        if (bank1_register == 0) bank1_register = 1;
    } else if (address < 0x6000) {
        bank2_register = value & 0x3;
    } else if (address < 0x8000) {
        rom_mode = (value & 1) == 0;
    }

    rom_bank = static_cast<u16>(((bank2_register << 5) | bank1_register) % rom.size());
    ram_bank = (rom_mode || ram.empty()) ? 0 : bank2_register % ram.size();
}

void MBC1::write_ram(const u16 address, const u8 value) {
    if (!ram.empty() && ram_enabled) {
        ram[ram_bank][address - 0xA000] = value;
    }
}

void MBC1::save(std::ostream &out) const {
    for (const auto &bank: ram) {
        out.write(reinterpret_cast<const char *>(bank.data()), bank.size());
    }
}

void MBC1::load(std::istream &in) {
    for (auto &bank: ram) {
        in.read(reinterpret_cast<char *>(bank.data()), bank.size());
    }
}

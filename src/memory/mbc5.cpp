#include "mbc5.h"

#include <istream>
#include <ostream>

MBC5::MBC5(RomBanks rom, const u16 ram_banks) : rom(std::move(rom)), ram(ram_banks) {
}

u8 MBC5::read_rom(const u16 address) const {
    if (address < 0x4000) {
        return rom[0][address];
    }
    return rom[rom_bank][address - 0x4000];
}

u8 MBC5::read_ram(const u16 address) const {
    if (ram.empty() || !ram_enabled) {
        return 0xFF;
    }
    return ram[ram_bank][address - 0xA000];
}

void MBC5::write(const u16 address, const u8 value) {
    if (address < 0x2000) {
        ram_enabled = (value & 0x0F) == 0xA;
    } else if (address < 0x3000) {
        rom_bank_low = value;
    } else if (address < 0x4000) {
        rom_bank_high = value & 0x1;
    } else if (address < 0x6000) {
        ram_bank = ram.empty() ? 0 : (value & 0x0F) % ram.size();
    }
    rom_bank = ((static_cast<u16>(rom_bank_high) << 8) | rom_bank_low) % rom.size();
}

void MBC5::write_ram(const u16 address, const u8 value) {
    if (!ram.empty() && ram_enabled) {
        ram[ram_bank][address - 0xA000] = value;
    }
}

void MBC5::save(std::ostream &out) const {
    for (auto &bank: ram) {
        out.write(reinterpret_cast<const char *>(bank.data()), bank.size());
    }
}

void MBC5::load(std::istream &in) {
    for (auto &bank: ram) {
        in.read(reinterpret_cast<char *>(bank.data()), bank.size());
    }
}

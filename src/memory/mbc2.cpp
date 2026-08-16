#include "mbc2.h"

#include <istream>
#include <ostream>

MBC2::MBC2(RomBanks rom) : rom(std::move(rom)) {}

u8 MBC2::read_rom(const u16 address) const {
    if (address < 0x4000) {
        return rom[0][address];
    }
    return rom[rom_bank][address - 0x4000];
}

u8 MBC2::read_ram(const u16 address) const {
    if (!ram_enabled) {
        return 0xFF;
    }
    return ram[(address - 0xA000) % 0x200] | 0xF0;
}

void MBC2::write(const u16 address, const u8 value) {
    if (address >= 0x4000) {
        return;
    }
    // Bit 8 of the address distinguishes the two registers sharing this window: set
    // selects the ROM bank register, clear selects RAM-enable.
    if ((address & 0x100) != 0) {
        rom_bank = (value & 0xF) % rom.size();
        if (rom_bank == 0) rom_bank = 1;
    } else {
        ram_enabled = (value & 0x0F) == 0x0A;
    }
}

void MBC2::write_ram(const u16 address, const u8 value) {
    if (!ram_enabled) {
        return;
    }
    ram[(address - 0xA000) % 0x200] = value & 0x0F;
}

void MBC2::save(std::ostream &out) const {
    out.write(reinterpret_cast<const char *>(ram.data()), ram.size());
}

void MBC2::load(std::istream &in) {
    in.read(reinterpret_cast<char *>(ram.data()), ram.size());
}

#ifndef BIOGB_ROM_ONLY_H
#define BIOGB_ROM_ONLY_H
// save()/load() are defined inline below (no .cpp for this class), so this needs the full
// class definitions, not just the <iosfwd> forward declarations mbc1.h/mbc2.h get away with.
#include <istream>
#include <ostream>

#include "rom_types.h"


class RomOnly {
public:
    RomOnly() = default;
    explicit RomOnly(RomBanks rom, const u16 ram_banks): rom(std::move(rom)), ram(ram_banks) {};

    [[nodiscard]] u8 read_rom(const u16 address) const {
        if (address < 0x4000) {
            return rom[0][address];
        }
        return rom[1][address - 0x4000];
    }

    [[nodiscard]] u8 read_ram(const u16 address) const {
        if (ram.empty()) {
            return 0xFF;
        }
        return ram[0][address - 0xA000];
    }

    void write(const u16 address, const u8 value) {}

    void write_ram(const u16 address, const u8 value) {
        if (!ram.empty()) {
            ram[0][address - 0xA000] = value;
        }
    }

    void save(std::ostream &out) const {
        for (const auto &bank: ram) {
            out.write(reinterpret_cast<const char *>(bank.data()), bank.size());
        }
    }

    void load(std::istream &in) {
        for (auto &bank: ram) {
            in.read(reinterpret_cast<char *>(bank.data()), bank.size());
        }
    }

private:
    RomBanks rom{};
    RamBanks ram{};
};


#endif //BIOGB_ROM_ONLY_H

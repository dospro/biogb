#ifndef BIOGB_MBC1_H
#define BIOGB_MBC1_H
// Forward-declares std::ostream/istream, enough for reference params in a declaration-only
// header; the definitions live in mbc1.cpp, which includes <ostream>/<istream> instead.
#include <iosfwd>

#include "../macros.h"
#include "rom_types.h"


class MBC1 {
public:
    MBC1(RomBanks rom, u16 ram_banks);

    [[nodiscard]] u8 read_rom(u16 address) const;
    [[nodiscard]] u8 read_ram(u16 address) const;

    void write(u16 address, u8 value);
    void write_ram(u16 address, u8 value);

    void save(std::ostream &out) const;
    void load(std::istream &in);

private:
    RomBanks rom{};
    RamBanks ram{};

    u16 rom_bank = 1;
    u16 ram_bank = 0;

    bool rom_mode = true;
    bool ram_enabled = false;
    u8 bank1_register = 1;
    u8 bank2_register = 0;
};


#endif //BIOGB_MBC1_H

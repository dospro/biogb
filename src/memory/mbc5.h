#ifndef BIOGB_MBC5_H
#define BIOGB_MBC5_H

#include <iosfwd>

#include "rom_types.h"


class MBC5 {
public:
    MBC5(RomBanks rom, u16 ram_banks);

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

    bool ram_enabled = false;
    u8 rom_bank_low = 1;
    u8 rom_bank_high = 0;

};


#endif //BIOGB_MBC5_H

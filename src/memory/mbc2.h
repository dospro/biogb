#ifndef BIOGB_MBC2_H
#define BIOGB_MBC2_H
#include <array>
// Forward-declares std::ostream/istream, enough for reference params in a declaration-only
// header; the definitions live in mbc2.cpp, which includes <ostream>/<istream> instead.
#include <iosfwd>

#include "../macros.h"
#include "rom_types.h"


class MBC2 {
public:
    explicit MBC2(RomBanks rom);

    [[nodiscard]] u8 read_rom(u16 address) const;
    [[nodiscard]] u8 read_ram(u16 address) const;

    void write(u16 address, u8 value);
    void write_ram(u16 address, u8 value);

    void save(std::ostream &out) const;
    void load(std::istream &in);

private:
    RomBanks rom{};
    std::array<u8, 0x200> ram{};

    u16 rom_bank = 1;
    bool ram_enabled = false;
};


#endif //BIOGB_MBC2_H

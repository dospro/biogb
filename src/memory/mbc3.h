#ifndef BIOGB_MBC3_H
#define BIOGB_MBC3_H
#include <iosfwd>

#include "../macros.h"
#include "rom_types.h"


struct RTC_Regs {
    u8 selected_register;
    bool selected;
    u8 latch;
    u8 sec;
    u8 min;
    u8 hr;
    u8 dl;
    u8 dh;
};

class MBC3 {
public:
    MBC3(RomBanks rom, u16 ram_banks);

    [[nodiscard]] u8 read_rom(u16 address) const;

    [[nodiscard]] u8 read_ram(u16 address) const;

    void write(u16 address, u8 value);

    void write_ram(u16 address, u8 value);

    void update_rtc(int cycles);

    void save(std::ostream &out) const;

    void load(std::istream &in);

    void save_rtc(std::ostream &out) const;
    void load_rtc(std::istream &in);

private:
    void tick_rtc();

    RomBanks rom{};
    RamBanks ram{};

    bool ram_enabled = false;
    u16 rom_bank = 1;
    u16 ram_bank = 0;

    int rtc_cycle_accumulator = 0;
    RTC_Regs rtc_snapshot{};
    RTC_Regs rtc_real{};
};


#endif //BIOGB_MBC3_H

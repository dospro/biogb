#include "mbc3.h"

#include <chrono>
#include <print>

MBC3::MBC3(RomBanks rom, const u16 ram_banks) : rom(std::move(rom)), ram(ram_banks) {
    const auto now = std::chrono::system_clock::now();
    const auto now_time_t = std::chrono::system_clock::to_time_t(now);
    const auto *local_tm = std::localtime(&now_time_t);

    rtc_real.sec = local_tm->tm_sec;
    rtc_real.min = local_tm->tm_min;
    rtc_real.hr = local_tm->tm_hour;
    rtc_real.dl = local_tm->tm_wday;
}

u8 MBC3::read_rom(const u16 address) const {
    if (address < 0x4000) {
        return rom[0][address];
    }
    return rom[rom_bank][address - 0x4000];
}

u8 MBC3::read_ram(const u16 address) const {
    if (rtc_snapshot.selected && ram_enabled) {
        switch (rtc_snapshot.selected_register) {
            case 0x8: return rtc_snapshot.sec;
            case 0x9: return rtc_snapshot.min;
            case 0xA: return rtc_snapshot.hr;
            case 0xB: return rtc_snapshot.dl;
            case 0xC: return rtc_snapshot.dh;
            default: return 0xFF;
        }
    }
    if (ram.empty() || !ram_enabled) {
        return 0xFF;
    }
    return ram[ram_bank][address - 0xA000];
}

void MBC3::write(const u16 address, u8 value) {
    if (address < 0x2000) {
        ram_enabled = (value & 0x0F) == 0x0A;
    } else if (address < 0x4000) {
        rom_bank = (value & 0x7F) % rom.size();
        if (rom_bank == 0) rom_bank = 1;
    } else if (address < 0x6000) {
        switch (value) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
                ram_bank = ram.empty() ? 0 : value % ram.size();
                rtc_snapshot.selected = false;
                break;
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
                rtc_snapshot.selected_register = value;
                rtc_snapshot.selected = true;
                break;
            default:
                std::println("Send MBC3 command: {:x}", value);
                break;
        }
    } else if (address >= 0x6000 && address < 0x8000) {
        const u8 prev_latch = rtc_snapshot.latch;
        rtc_snapshot.latch = value;

        if (prev_latch == 0 && value == 1) {
            rtc_snapshot.dh = rtc_real.dh;
            rtc_snapshot.dl = rtc_real.dl;
            rtc_snapshot.hr = rtc_real.hr;
            rtc_snapshot.min = rtc_real.min;
            rtc_snapshot.sec = rtc_real.sec;
        }
    }
}

void MBC3::write_ram(const u16 address, const u8 value) {
    if (rtc_snapshot.selected && ram_enabled) {
        switch (rtc_snapshot.selected_register) {
            case 0x8: rtc_real.sec = value; break;
            case 0x9: rtc_real.min = value; break;
            case 0xA: rtc_real.hr = value;  break;
            case 0xB: rtc_real.dl = value;  break;
            case 0xC: rtc_real.dh = value;  break;
            default: std::println("Sending rtc command to address {}", rtc_snapshot.selected_register);
        }
    } else {
        if (!ram.empty() && ram_enabled) {
            ram[ram_bank][address - 0xA000] = value;
        }
    }
}

void MBC3::update_rtc(const int cycles) {
    constexpr int cycles_per_second = 4194304;
    rtc_cycle_accumulator += cycles;
    while (rtc_cycle_accumulator >= cycles_per_second) {
        rtc_cycle_accumulator -= cycles_per_second;
        tick_rtc();
    }
}

void MBC3::tick_rtc() {
    if ((rtc_real.dh >> 6) & 1) {
        return; // halted
    }

    if (++rtc_real.sec < 60) return;
    rtc_real.sec = 0;

    if (++rtc_real.min < 60) return;
    rtc_real.min = 0;

    if (++rtc_real.hr < 24) return;
    rtc_real.hr = 0;

    if (++rtc_real.dl != 0) return;
    if ((rtc_real.dh & 1) == 0) {
        rtc_real.dh |= 1;
    } else {
        rtc_real.dh |= 0x80;
        rtc_real.dh &= 0xFE;
    }
}

void MBC3::save(std::ostream &out) const {
    for (auto &bank: ram) {
        out.write(reinterpret_cast<const char *>(bank.data()), bank.size());
    }
}

void MBC3::load(std::istream &in) {
    for (auto &bank: ram) {
        in.read(reinterpret_cast<char *>(bank.data()), bank.size());
    }
}

void MBC3::save_rtc(std::ostream &out) const {
    out.write(reinterpret_cast<const char *>(&rtc_real), sizeof(RTC_Regs));
    out.write(reinterpret_cast<const char *>(&rtc_snapshot), sizeof(RTC_Regs));
}

void MBC3::load_rtc(std::istream &in) {
    in.read(reinterpret_cast<char *>(&rtc_real), sizeof(RTC_Regs));
    in.read(reinterpret_cast<char *>(&rtc_snapshot), sizeof(RTC_Regs));
}

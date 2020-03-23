#ifndef BIOGB_ROMLOADER_H
#define BIOGB_ROMLOADER_H

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include "../macros.h"

using RomBank = std::array<u8, 0x4000>;

struct RomType {
    int id;
    char name[128];
};

struct BanksInfo {
    int id;
    int size;
    int banks;
};

enum class MBCTypes {
    RomOnly,
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    Generic
};

class RomLoader {
public:
    RomLoader(const std::string &file_name);
    std::vector<RomBank> get_rom();
    bool is_color();
    bool has_timer();
    int get_ram_banks();
    MBCTypes get_mbc_type();

private:
    void read_header();
    static MBCTypes calculate_mbc_type(u8 mbc_id);
    static bool has_mbc_timer(u8 mbc_id);
    int calculate_rom_banks(u8 rom_size_id);
    int calculate_ram_banks(u8 ram_size_id);
    void load_rom(int banks);
    std::vector<RomBank> rom;
    std::ifstream file;
    std::string name;
    bool color;
    bool with_timer;
    MBCTypes mbc;
    int rom_banks;
    int ram_banks;
};


#endif //BIOGB_ROMLOADER_H

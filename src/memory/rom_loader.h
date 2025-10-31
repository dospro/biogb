#ifndef BIOGB_ROM_LOADER_H
#define BIOGB_ROM_LOADER_H

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include "../macros.h"

using RomBank = std::array<u8, 0x4000>;

struct RomType {
    int id;
    std::string_view name;
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
    explicit RomLoader(std::string_view file_name);
    [[nodiscard]] std::vector<RomBank> get_rom();
    [[nodiscard]] bool is_color() const;
    [[nodiscard]] bool has_timer() const;
    [[nodiscard]] int get_ram_banks() const;
    [[nodiscard]] MBCTypes get_mbc_type() const;

private:
    void read_header();
    static constexpr MBCTypes calculate_mbc_type(u8 mbc_id) noexcept;
    static constexpr bool has_mbc_timer(u8 mbc_id) noexcept;
    int calculate_rom_banks(u8 rom_size_id);
    int calculate_ram_banks(u8 ram_size_id);
    void load_rom(int banks);
    std::vector<RomBank> rom{};
    std::ifstream file{};
    std::string name{};
    bool color{};
    bool with_timer{};
    MBCTypes mbc{};
    int rom_banks{};
    int ram_banks{};
};


#endif  // BIOGB_ROM_LOADER_H

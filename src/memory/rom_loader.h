#ifndef BIOGB_ROM_LOADER_H
#define BIOGB_ROM_LOADER_H

#include <vector>
#include <array>
#include <optional>
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

enum class ColorSupport : u8 {
    None, // DMG
    Enhanced, // 0x80 CGB or DGM
    Required, // 0xC0 CGB only
};

[[nodiscard]] constexpr std::string_view to_string(const ColorSupport support) {
    switch (support) {
        case ColorSupport::None: return "DMG";
        case ColorSupport::Enhanced: return "Game Boy Color supported";
        case ColorSupport::Required: return "Game Boy Color only ROM";
    }
    return "Unknown";
}

struct CartridgeSupport {
    ColorSupport color{ColorSupport::None};
    bool sgb{false};
};

/**
 * @brief The machine the emulator presents itself as.
 *
 * Decided once, before the CPU starts, from what the cart supports and what the user asked
 * for. Everything downstream reads this and nothing else: boot register values, whether color
 * hardware exists, whether the SGB packet receiver is live, and the output resolution.
 */
enum class ConsoleModel : u8 {
    DMG,
    SGB,
    CGB,
};

[[nodiscard]] constexpr std::string_view to_string(const ConsoleModel model) {
    switch (model) {
        case ConsoleModel::DMG: return "Original Monochrome Game Boy";
        case ConsoleModel::SGB: return "Super Game Boy";
        case ConsoleModel::CGB: return "Game Boy Color";
    }
    return "Unknown";
}

/**
 * @brief Picks the machine to emulate.
 *
 * @p preference is the user overriding us from the command line, so it is absolute: no
 * fallback, no warning. Passing nothing means "pick for me", and we prefer an SGB whenever the
 * cart supports one -- which is what the real hardware does, so a CGB-enhanced cart that also
 * carries SGB support runs monochrome with a border rather than in color.
 */
[[nodiscard]] constexpr ConsoleModel selected_console_model(
    const CartridgeSupport support,
    const std::optional<ConsoleModel> preference = std::nullopt
) {
    if (preference) return *preference;

    // A rom that requires GBC cannot run on a SGB at all.
    if (support.color == ColorSupport::Required) return ConsoleModel::CGB;
    if (support.sgb) return ConsoleModel::SGB;
    if (support.color == ColorSupport::Enhanced) return ConsoleModel::CGB;
    return ConsoleModel::DMG;
}

class RomLoader {
public:
    explicit RomLoader(std::string_view file_name);
    [[nodiscard]] std::vector<RomBank> get_rom();
    [[nodiscard]] CartridgeSupport get_support() const;
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
    bool with_timer{};
    CartridgeSupport support{};
    MBCTypes mbc{};
    int rom_banks{};
    int ram_banks{};
};


#endif  // BIOGB_ROM_LOADER_H

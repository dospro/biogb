#ifndef BIOGB_ROM_TYPES_H
#define BIOGB_ROM_TYPES_H
#include <array>
#include <vector>

#include "../macros.h"

using RomBank = std::array<u8, 0x4000>;
using RamBank = std::array<u8, 0x2000>;
using RomBanks = std::vector<RomBank>;
using RamBanks = std::vector<RamBank>;

#endif  // BIOGB_ROM_TYPES_H

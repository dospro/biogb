#ifndef BIOGB_SGB_H
#define BIOGB_SGB_H
#include <cstdint>
#include <functional>
#include <mdspan>
#include <ostream>
#include <vector>

#include "macros.h"


class BitPacker {
public:
    BitPacker() = default;

    ~BitPacker() = default;

    void add_bit(const uint8_t bit) {
        if (bit_index == 128 && bit == 0) {
            is_packet_full = true;
            return;
        }
        if (bit_index >= 128) {
            return;
        }
        const size_t byte_index = bit_index >> 3;
        const size_t bit_position = bit_index & 7;
        buffer[byte_index] |= bit << bit_position;
        bit_index++;
    }

    [[nodiscard]] bool is_full() const { return is_packet_full; }

    [[nodiscard]] uint8_t get_byte(const size_t index) const {
        if (index >= 16) {
            std::println("Index out of bounds: {}", index);
            return 0;
        }
        return buffer[index];
    }

    [[nodiscard]] std::array<u8, 16> get_packet() const {
        return buffer;
    }

    void reset() {
        buffer.fill(0);
        is_packet_full = false;
        bit_index = 0;
    }

private:
    std::array<u8, 16> buffer{};
    size_t bit_index = 0;
    bool is_packet_full = false;
};

enum CommandType: u8 {
    PAL01,
    PAL23,
    PAL03,
    PAL12,
    ATTR_BLK,
    ATTR_LIN,
    ATTR_DIV,
    ATTR_CHR,
    SOUND,
    SND_TRN,
    PAL_SET,
    PAL_TRN,
    ATRC_EN,
    TEST_EN,
    ICON_EN,
    DATA_SND,
    DATA_TRN,
    MLT_REQ,
    JUMP,
    CHR_TRN,
    PCT_TRN,
    ATTR_TRN,
    ATTR_SET,
    MASK_EN,
    OBJ_TRN,
    PAL_PRI
};

struct MultiPlayer {
    bool changing_joyp = false;
    u8 players = 0;
    u8 current_player = 0;
};

class SGB {
public:
    using Packet = std::array<u8, 16>;
    using PacketListener = std::function<void(u8 command, std::vector<Packet> packet)>;

    SGB() = default;

    ~SGB() = default;

    void add_packet_listener(const PacketListener& listener) { listeners.push_back(listener);};

    void start_transfer_mode();

    void send_bit(uint8_t bit);

    [[nodiscard]] bool is_sgb_transfer_mode() const;

    [[nodiscard]] bool mlt_is_active() const;

    [[nodiscard]] u8 mlt_get_current_player() const;

    void mlt_change_joyp(bool value);
    void write_sgb_palette(size_t index, u16 data);

private:

    void emit_command() const;
    std::vector<PacketListener> listeners{};
    bool sgb_transfer_mode = false;
    u8 length = 0;
    u8 command = 0;
    BitPacker packer{};
    std::vector<Packet> packets{};
    MultiPlayer mlt_req{};

    std::array<u16, 8 * 16> palettes_data{};
    std::mdspan<u16, std::extents<size_t, 8, 16> > palettes{palettes_data.data()};

    std::array<u16, 512 * 16> system_palettes_data{};
    std::mdspan<u16, std::extents<size_t, 512, 16> > system_palettes{system_palettes_data.data()};
};


#endif //BIOGB_SGB_H

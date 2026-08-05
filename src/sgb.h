#ifndef BIOGB_SGB_H
#define BIOGB_SGB_H
#include <cstdint>
#include <functional>
#include <mdspan>
#include <optional>
#include <ostream>
#include <vector>

#include "macros.h"


class BitPacker {
public:
    BitPacker() = default;

    ~BitPacker() = default;

    /**
     * @brief Appends one data bit to the packet being assembled.
     *
     * Bits arrive LSB-first within each byte, so bit N lands in byte N/8 at position N%8.
     * The 129th call is the protocol's stop bit: it is not stored, it only marks the packet
     * complete. Its value is ignored -- real games always send 0, but accepting either value
     * keeps the receiver from stalling (see "Stop bit value" in docs/SGB_implementation.md).
     * Bits past that point are dropped with a warning, which can only happen if a caller
     * forgets to take_packet().
     */
    void add_bit(const uint8_t bit) {
        if (bit_index >= 128) {
            if (is_packet_full) {
                std::println("SGB Warning: Bit index out of bounds: {}", bit_index);
            }
            is_packet_full = true;
            return;
        }
        const size_t byte_index = bit_index >> 3;
        const size_t bit_position = bit_index & 7;
        buffer[byte_index] |= bit << bit_position;
        bit_index++;
    }

    /**
     * @brief Removes the finished packet, if one is ready.
     *
     * The only way to read the buffer. Taking the packet also resets the accumulator, so the
     * caller cannot leave it full by accident -- the next add_bit() starts a fresh packet.
     *
     * @return The 16 assembled bytes, or nullopt while the packet is still incomplete.
     */
    [[nodiscard]] std::optional<std::array<u8, 16>> take_packet() {
        if (!is_packet_full) return std::nullopt;
        const auto result = buffer;
        reset();
        return result;
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

    /**
     * @brief Feeds a JOYP (0xFF00) write into the SGB packet receiver.
     *
     * Only bits 4-5 are significant. Every token is a pulse followed by a return to neutral
     * (0x30), so the decoding happens on the 0x30 write and looks at whatever preceded it:
     *
     *     0x00 -> 0x30   reset pulse, begins a packet
     *     0x10 -> 0x30   data bit 1
     *     0x20 -> 0x30   data bit 0
     *
     * Any other sequence -- including a repeated 0x30 -- is a no-op; the receiver is
     * best-effort and never blocks waiting for a well-formed cadence. latch_value is recorded
     * before any early return, so every write updates it exactly once.
     *
     * @param a_value Raw value written to JOYP.
     */
    void write(u8 a_value);

    [[nodiscard]] bool is_sgb_transfer_mode() const;

    [[nodiscard]] bool mlt_is_active() const;

    [[nodiscard]] u8 mlt_get_current_player() const;

    void mlt_change_joyp(bool value);
    void write_sgb_system_palette(size_t index, u16 data);
    void write_sgb_tile_map();
    void write_sgb_palette();
    void write_sgb_tile_data();
    void write_sgb_system_palette();
    [[nodiscard]] std::span<u32> get_sgb_video_buffer();

    void write_buffer(const size_t index, const u8 data) { vram_transfer_buffer[index % 0x1000] = data; }

private:
    /**
     * @brief Collects a finished packet, dispatching the command once its group is complete.
     *
     * A command spans 1-7 packets. Byte 0 of the first one is a header: command in the upper
     * 5 bits, total packet count for the group in the lower 3. Later packets carry data only.
     * Transfer mode ends with every packet, so each one needs its own reset pulse to start.
     *
     * A header declaring zero packets is malformed and drops the group, so the next packet is
     * read as a fresh header instead of accumulating forever.
     *
     * @param packet The 16 bytes just assembled by the BitPacker.
     */
    void commit_packet(const Packet &packet);
    void emit_command() const;
    void handle_command();

    u8 latch_value = 0xFF; // Used to save a value before the 0x30 write.

    std::array<u32, 256 * 224> buffer{};
    std::vector<PacketListener> listeners{};

    bool sgb_transfer_mode = false;
    u8 length = 0;
    u8 command = 0;
    BitPacker packer{};
    std::vector<Packet> packets{};
    MultiPlayer mlt_req{};

    std::array<u8, 0x1000> vram_transfer_buffer{};

    std::array<u16, 8 * 16> palettes_data{};
    std::mdspan<u16, std::extents<size_t, 8, 16> > palettes{palettes_data.data()};

    std::array<u16, 512 * 16> system_palettes_data{};
    std::mdspan<u16, std::extents<size_t, 512, 16> > system_palettes{system_palettes_data.data()};

    std::array<u8, 0x740> tile_map{};
    std::array<u8, 0x2000> tile_data{};
};


#endif //BIOGB_SGB_H

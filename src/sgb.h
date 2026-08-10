#ifndef BIOGB_SGB_H
#define BIOGB_SGB_H
#include <mdspan>
#include <optional>
#include <array>
#include <span>
#include <print>
#include <vector>
#include <algorithm>

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
    void add_bit(const u8 bit) {
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

/**
 * @brief What a completed *_TRN command is waiting to do with the next captured screen.
 *
 * A TRN command does not carry its payload in the packets. It announces that the payload is
 * sitting on the screen, so the work has to wait for a frame boundary -- this records which
 * work that is in the meantime.
 */
enum class PendingTransfer : u8 {
    None,
    TileData,    // CHR_TRN
    TileMap,     // PCT_TRN -- carries the border map *and* its palettes
    SystemPalette, // PAL_TRN
};

/**
 * @brief What the SNES shows in place of the live Game Boy window (MASK_EN).
 *
 * Display side only. The Game Boy keeps rendering and the SGB keeps sampling that signal
 * masking the picture while a *_TRN payload is on screen is the command's main purpose, so
 * the capture path cannot be affected by it.
 */
enum class ScreenMask : u8 {
    Cancel,         // 0 - live picture
    Freeze,         // 1 - keep showing the stored picture
    BlankBlack,     // 2
    BlankColor0,    // 3 - solid SGB color 0
};

class MultiplayerState {
public:
    /**
     * Rule 1: every P15 0->1 edge on JOYP advances the active player. No exceptions.
     */
    void advance_player() { current = (current + 1) & (player_count - 1); }

    /**
     * Rule 2: A new MLT_REQ moves the current player number
     */
    void set_player_count(const u8 count) {
        player_count = count;
        current &= (count - 1);
    }

    [[nodiscard]] u8 active_player() const { return current; }

    /**
     * What P1 register read back so ROM can detect the number of players.
     */
    [[nodiscard]] u8 id_nibble() const { return 0xF - current; }
    [[nodiscard]] bool is_multiplayer() const { return player_count > 1; }

private:
    u8 player_count = 1;
    u8 current = 0;
};

class SGB {
public:
    using Packet = std::array<u8, 16>;

    SGB() = default;

    ~SGB() = default;

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

    /**
    * @brief Advances any pending *_TRN transfer. Call once per frame, at the frame boundary.
    *
    * A no-op unless a TRN command is waiting. @p screen is the display's 160x144 buffer of
    * decoded 2bpp pixel values for the frame that just finished.
    */
    void run_pending_transfer(std::span<const u8> screen);

    /**
    * @brief Builds the picture the TV actually shows: the SGB border with the Game Boy
    *        window composited into the middle, under the current MASK_EN state.
    *
    * Call once per frame, after the frame has finished rendering. @p gb_frame is the
    * display's 160x144 buffer of RGB888 pixels for that frame.
    */
    [[nodiscard]] std::span<const u32> compose_frame(std::span<const u32> gb_frame);

    [[nodiscard]] bool mlt_is_multiplayer() const { return multiplayer_req.is_multiplayer(); }
    [[nodiscard]] u8 mlt_id_nibble() const { return multiplayer_req.id_nibble(); }

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

    void handle_command();

    /**
     * @brief Reassembles the 4KB transfer payload from the DMG's rendered screen.
     *
     * The SGB has no bus access to DMG VRAM -- it only samples the video output. A compliant
     * ROM arranges the screen so that reading it in raster order reproduces the bytes it wants
     * to send. @p screen is the display's 160x144 buffer of decoded 2bpp
     * pixel values, which is exactly 256 tiles' worth of data at 16 bytes per tile.
     */
    void capture_screen(std::span<const u8> screen);

    void schedule_transfer(PendingTransfer transfer);

    void write_sgb_tile_map();
    void write_sgb_palette();
    void write_sgb_tile_data();
    void write_sgb_system_palette();
    /**
    * @brief Reads one little-endian RGB555 color out of the current packet at @p byte_offset.
    */
    [[nodiscard]] u16 packet_color(size_t byte_offset) const;
    /**
    * @brief Applies a PAL01/PAL23/PAL03/PAL12 packet to the two palettes it names.
    *
    * All four commands share one 16-byte layout, differing only in which pair of palettes they
    * target: bytes 1-2 hold color 0, bytes 3-8 are colors 1-3 of @p first, and bytes 9-14 are
    * colors 1-3 of @p second.
    */
    void apply_pal_command(size_t first, size_t second);

    void render_border();

    [[nodiscard]] static constexpr u32 to_rgb888(const u16 color);

    MultiplayerState multiplayer_req{};
    ScreenMask screen_mask{ScreenMask::Cancel};

    u8 latch_value = 0xFF; // Used to save a value before the 0x30 write.

    std::array<u32, 256 * 224> composed_frame{};

    // The SNES's own copy of the last picture it displayed. MASK_EN's Freeze mode holds this
    // instead of the live one; every other mode keeps it current.
    std::array<u32, 160 * 144> stored_frame{};

    bool sgb_transfer_mode = false;
    u8 length = 0;
    u8 command = 0;
    BitPacker packer{};
    std::vector<Packet> packets{};

    // The one argument byte a *_TRN command carries (CHR_TRN's bank-select bit today).
    // Copied out when the command arrives, because `packets` is cleared as soon as
    // handle_command() returns -- long before the deferred transfer actually runs.
    u8 transfer_arg = 0;
    PendingTransfer pending = PendingTransfer::None;
    u8 frames_until_capture = 0;

    std::array<u8, 0x1000> vram_transfer_buffer{};

    std::array<u16, 8 * 16> palettes_data{};
    std::mdspan<u16, std::extents<size_t, 8, 16> > palettes{palettes_data.data()};

    std::array<u16, 512 * 16> system_palettes_data{};
    std::mdspan<u16, std::extents<size_t, 512, 16> > system_palettes{system_palettes_data.data()};

    std::array<u8, 0x740> tile_map{};
    std::array<u8, 0x2000> tile_data{};
};


#endif //BIOGB_SGB_H

#include "sgb.h"


void SGB::write(const u8 a_value) {
    const u8 value = a_value & 0x30;
    const u8 previous = latch_value;
    latch_value = value;

    if ((previous & 0x20) == 0 && (value & 0x20) != 0) {
        multiplayer_req.advance_player();
    }

    if (value != 0x30) return;

    if (previous == 0x00) {
        sgb_transfer_mode = true;
        packer.reset();
        return;
    }

    if (previous != 0x10 && previous != 0x20) return;
    if (!sgb_transfer_mode) return;

    packer.add_bit(previous == 0x10 ? 1 : 0);
    if (const auto packet = packer.take_packet()) {
        commit_packet(*packet);
    }
}

void SGB::commit_packet(const Packet &packet) {
    sgb_transfer_mode = false;

    if (length == 0) {
        // First packet of a group carries the header: command in the upper 5 bits, packet count
        // for the whole group in the lower 3.
        length = packet[0] & 7;
        command = packet[0] >> 3;
    }

    packets.emplace_back(packet);

    if (length == 0) {
        // The header declared a group of zero packets, which is malformed. Drop it so the next
        // packet is read as a fresh header instead of accumulating forever.
        std::println("SGB Warning: header declares a zero-length group, dropping packet");
        packets.clear();
        return;
    }

    if (packets.size() < length) return;

    handle_command();
    packets.clear();
    length = 0;
    command = 0;
}

void SGB::handle_command() {
    switch (command) {
        case PAL01: std::println("PAL01"); apply_pal_command(0, 1); break;
        case PAL23: std::println("PAL23"); apply_pal_command(2, 3); break;
        case PAL03: std::println("PAL03"); apply_pal_command(0, 3); break;
        case PAL12: std::println("PAL12"); apply_pal_command(1, 2); break;
        case PAL_SET:
            std::println("PAL_SET");
            for (size_t palette_number = 0; palette_number < 4; ++palette_number) {
                const auto palette_id = (packets[0][palette_number * 2 + 1] |
                                         (packets[0][palette_number * 2 + 2] << 8)) & 0x1FF;
                for (size_t i = 0; i < 4; ++i) {
                    palettes[palette_number, i] = system_palettes[palette_id, i];
                }
            }
            if ((packets[0][9] & 0x40) != 0) {
                screen_mask = ScreenMask::Cancel;
            }
            break;
        case PAL_TRN:
            std::println("PAL_TRN");
            schedule_transfer(PendingTransfer::SystemPalette);
            break;
        case DATA_SND:
            std::println("DATA_SND command");
            std::println("Writing into SNES address {:x}", packets[0][1] | (packets[0][2] << 8));
            break;
        case DATA_TRN:
            std::println("DATA_TRN command");
            std::println("VRAM transfer into SNES address {:x}", packets[0][1] | (packets[0][2] << 8));
            break;
        case MLT_REQ:
            std::println("MLT_REQ command");
            switch (packets[0][1] & 3) {
                case 0: multiplayer_req.set_player_count(1);
                    break;
                case 1: multiplayer_req.set_player_count(2);
                    break;
                case 3: multiplayer_req.set_player_count(4);
                    break;
                default: std::println("MLT_REQ: reserved player count 0b10, ignored");
                    break;
            }
            break;
        case CHR_TRN:
            std::println("CHR_TRN command");
            schedule_transfer(PendingTransfer::TileData);
            break;
        case PCT_TRN:
            std::println("PCT_TRN command");
            schedule_transfer(PendingTransfer::TileMap);
            break;
        case ATTR_TRN:
            std::println("ATTR_TRN command (Unhandled");
            break;
        case MASK_EN:
            std::println("MASK_EN command");
            screen_mask = static_cast<ScreenMask>(packets[0][1] & 3);
            break;
        default:
            std::println("Unhandled SGB command: {:x}", command);
            break;
    }
}

void SGB::capture_screen(const std::span<const u8> screen) {
    const std::mdspan<const u8, std::extents<size_t, 144, 160> > pixels{screen.data()};

    /*
     * The payload is 4096 bytes. A tile is 8x8 pixels at 2 bits per pixel, which is 128 bits,
     * or 16 bytes. So the payload is exactly 256 tiles worth of data.
     *
     * We have no way to read those bytes directly -- all we get is the finished picture. But
     * the picture was drawn *from* the bytes we are after, and drawing does not lose anything:
     * every pixel on screen still carries the 2 bits it was built from. So recovering the
     * payload means visiting the 256 tiles in the order they were laid out and squeezing each
     * 8x8 block of pixels back into the 16 bytes it came from.
     */
    for (size_t tile = 0; tile < 256; ++tile) {
        /*
         * `pixels` is a flat 144x160 grid with no notion of tiles, so first we turn a tile
         * number into the pixel coordinate of that tile's top-left corner.
         *
         * The screen is 160 pixels wide, which is 20 tiles. Counting tiles left to right and
         * top to bottom, `tile % 20` says how far along the current row we are and `tile / 20`
         * says how many rows are already behind us. Multiplying by 8 converts both from tiles
         * to pixels.
         *
         * Together they sweep the grid the same way the tiles were placed: step across 20
         * tiles, wrap down to the next row, repeat. 256 tiles fill 13 rows, so only the top
         * 104 pixel rows are ever read and the bottom of the screen is irrelevant here.
         */
        const auto tile_row_pos = (tile / 20) * 8;
        const auto tile_col_pos = (tile % 20) * 8;

        /*
         * Now rebuild the tile itself: 8 rows of 8 pixels, 2 bytes per row.
         *
         * The catch is that a pixel's 2 bits are not stored side by side. They are split across
         * the row's two bytes -- one byte holds bit 0 of all 8 pixels, the other holds bit 1 of
         * those same 8 pixels. That is why a single pass over the row feeds two accumulators
         * instead of one: each pixel we read contributes one bit to each byte.
         *
         * Inside a byte the leftmost pixel is the most significant bit, so pixel `tile_x` is
         * shifted by (7 - tile_x): pixel 0 lands in bit 7, pixel 7 in bit 0.
         */
        for (size_t tile_y = 0; tile_y < 8; ++tile_y) {
            u8 low_byte = 0;
            u8 high_byte = 0;
            for (size_t tile_x = 0; tile_x < 8; ++tile_x) {
                const auto pixel_2bpp = pixels[tile_row_pos + tile_y, tile_col_pos + tile_x];
                low_byte |= (pixel_2bpp & 1) << (7 - tile_x);
                high_byte |= ((pixel_2bpp >> 1) & 1) << (7 - tile_x);
            }
            /*
             * A row's two bytes are adjacent, low bits first, and the 8 rows follow one another,
             * so tile N owns bytes [N*16, N*16+16). The final write lands at
             * 255*16 + 7*2 + 1 = 4095, filling the buffer exactly: the loop bounds are what
             * keeps this in range, so no explicit clamp is needed.
             */
            vram_transfer_buffer[tile * 16 + tile_y * 2] = low_byte;
            vram_transfer_buffer[tile * 16 + tile_y * 2 + 1] = high_byte;
        }
    }
}

void SGB::run_pending_transfer(std::span<const u8> screen) {
    if (pending == PendingTransfer::None) return;
    if (frames_until_capture > 0) {
        --frames_until_capture;
        return;
    }
    capture_screen(screen);
    switch (pending) {
        case PendingTransfer::TileData:
            write_sgb_tile_data();
            break;
        case PendingTransfer::TileMap:
            write_sgb_tile_map();
            write_sgb_palette();
            break;
        case PendingTransfer::SystemPalette:
            write_sgb_system_palette();
            break;
        default:
            break;
    }
    pending = PendingTransfer::None;
}

void SGB::schedule_transfer(const PendingTransfer transfer) {
    pending = transfer;
    transfer_arg = packets[0][1];
    frames_until_capture = 1;
}

void SGB::write_sgb_tile_map() {
    for (size_t i = 0; i < 0x740; ++i) {
        tile_map[i] = vram_transfer_buffer[i];
    }
}

void SGB::write_sgb_palette() {
    // 3 palettes
    // 16 colors each
    // 2 bytes per color
    // Total 0x60 bytes
    for (size_t i = 0; i < 0x60; i += 2) {
        const u16 data = vram_transfer_buffer[0x800 + i] | (vram_transfer_buffer[0x800 + i + 1] << 8);
        const auto color_index = i / 2;
        const auto palette_number = (color_index >> 4) + 4;
        const auto palette_index = color_index & 0xF;
        palettes[palette_number, palette_index] = data;
    }
}

/*
 * CHR_TRN bit 0 selects which half of the tile set this payload fills:
 * 0 -> tiles 0-127, 1 -> tiles 128-255. Each half is one full 4KB payload.
 */
void SGB::write_sgb_tile_data() {
    const size_t destination = (transfer_arg & 1) ? 0x1000 : 0;
    std::ranges::copy(vram_transfer_buffer, tile_data.begin() + destination);
}

void SGB::write_sgb_system_palette() {
    /*
     * There are 512 system palettes
     * Each palette has 4 colors
     * Each color is 2 bytes
     * So, each palette is 8 bytes
     */
    for (size_t palette = 0; palette < 512; ++palette) {
        for (size_t index = 0; index < 4; ++index) {
            const auto buffer_index = palette * 8 + index * 2;
            const u16 data = vram_transfer_buffer[buffer_index] | (vram_transfer_buffer[buffer_index + 1] << 8);
            system_palettes[palette, index] = data;
        }
    }
}


u16 SGB::packet_color(const size_t byte_offset) const {
    return packets[0][byte_offset] | (packets[0][byte_offset + 1] << 8);
}

void SGB::apply_pal_command(const size_t first, const size_t second) {
    const u16 color0 = packet_color(1);
    for (size_t palette = 0; palette < 4; ++palette) {
        palettes[palette, 0] = color0;
    }

    for (size_t color = 1; color < 4; ++color) {
        palettes[first, color] = packet_color(1 + color * 2); // bytes 3, 5, 7
        palettes[second, color] = packet_color(7 + color * 2); // bytes 9, 11, 13
    }
}

void SGB::render_border() {
    /*
     * The plan
     *
     * First, go entry by entry in the map to get the data
     * Then get pattern
     * Get the palette and colors.
     *
     * Write it to the buffer;
     */

    std::mdspan buffer_view{composed_frame.data(), std::extents<size_t, 224, 256>{}};

    for (size_t y = 0; y < 28; ++y) {
        for (size_t x = 0; x < 32; ++x) {
            const auto tile_map_index = (y * 32 + x) * 2;
            const auto tile_map_entry = tile_map[tile_map_index] | (tile_map[tile_map_index + 1] << 8);

            const auto tile_index = tile_map_entry & 0xFF;
            const auto palette_index = (tile_map_entry >> 10) & 0x7;
            const bool x_flip = ((tile_map_entry >> 14) & 1) == 1;
            const bool y_flip = ((tile_map_entry >> 15) & 1) == 1;

            /*
             * Each tile is 32 bytes: 8x8 pixels of 16 colors each
             * So each row has 8 pixels.
             * Each pixel needs 4 bits (to get 16 colors),
             * so each row uses 4 bytes
             * These 4 bytes are divided in 2 blocks:
             * First come the first 2 bytes for all rows
             * Then come the second 2 bytes for all rows
             * Example:
             * byte 0 has bit0 of row 0
             * byte 1 has bit1 of row 0
             * byte 2 has bit0 of row 1
             * byte 3 has bit1 of row 1
             * and so on...
             * byte 16 has bit2 of row 0
             * byte 17 has bit3 of row 0
             * and so on
             *
             */
            for (size_t row = 0; row < 8; ++row) {
                const u8 byte0 = tile_data[tile_index * 32 + row * 2];
                const u8 byte1 = tile_data[tile_index * 32 + row * 2 + 1];
                const u8 byte2 = tile_data[tile_index * 32 + row * 2 + 16];
                const u8 byte3 = tile_data[tile_index * 32 + row * 2 + 17];

                const size_t dest_row = y_flip ? 7 - row : row;

                for (size_t small_x = 0; small_x < 8; ++small_x) {
                    const auto color_index = ((byte0 >> (7 - small_x)) & 1) |
                                             (((byte1 >> (7 - small_x)) & 1) << 1) |
                                             (((byte2 >> (7 - small_x)) & 1) << 2) |
                                             (((byte3 >> (7 - small_x)) & 1) << 3);
                    if (color_index == 0) continue;
                    const size_t dest_col = x_flip ? 7 - small_x : small_x;
                    buffer_view[y * 8 + dest_row, x * 8 + dest_col] = to_rgb888(palettes[palette_index, color_index]);
                }
            }
        }
    }
}

std::span<const u32> SGB::compose_frame(std::span<const u32> gb_frame) {
    /*
    * The SNES continuously buffers the picture it is showing; Freeze does not capture
    * anything, it simply stops refreshing that buffer. Keeping the copy current in every
    * other mask state is what reproduces the lag the hardware docs describe -- a ROM that
    * freezes immediately after drawing holds the *previous* picture, which is exactly why
    * it is told to wait a frame or two before freezing.
    *
    * Freezing before anything was ever stored leaves stored_frame zeroed and the window
    * goes black. Real hardware is documented as undefined there; black is our choice.
    */
    if (screen_mask != ScreenMask::Freeze) {
        std::ranges::copy(gb_frame, stored_frame.begin());
    }


    /*
     * Modes 2 and 3 replace the window with one flat color. Modes 0 and 1 both show
     * stored_frame. The only difference between them is whether it was refreshed above.
     * Color 0 is shared by all eight SGB palettes, so palette 0's entry is the cannonical copy.
     */
    const bool blanked = screen_mask == ScreenMask::BlankBlack || screen_mask == ScreenMask::BlankColor0;
    const u32 blank_color = screen_mask == ScreenMask::BlankBlack ? 0x000000u : to_rgb888(palettes[0, 0]);

    /*
    * The backdrop: what shows wherever no layer draws. Color 0 is shared across all eight
    * palettes, so palette 0's entry is the canonical copy. Laying it down first means the two
    * layers above only have to fill in what they actually cover.
    */
    std::ranges::fill(composed_frame, to_rgb888(palettes[0, 0]));

    const std::mdspan<const u32, std::extents<size_t, 144, 160> > stored_view{stored_frame.data()};
    std::mdspan composed_view{composed_frame.data(), std::extents<size_t, 224, 256>{}};

    constexpr size_t y_offset = (224 - 144) / 2;
    constexpr size_t x_offset = (256 - 160) / 2;

    for (size_t y = 0; y < 144; ++y) {
        for (size_t x = 0; x < 160; ++x) {
            composed_view[y + y_offset, x + x_offset] = blanked ? blank_color : stored_view[y, x];
        }
    }

    render_border();
    return composed_frame;
}

constexpr u32 SGB::to_rgb888(const u16 color) {
    const u32 red = color & 0x1F;
    const u32 green = (color >> 5) & 0x1F;
    const u32 blue = (color >> 10) & 0x1F;
    return (((red << 3) | (red >> 2)) << 16) | (((green << 3) | (green >> 2)) << 8) |
           ((blue << 3) | (blue >> 2));
}

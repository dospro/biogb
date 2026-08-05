#include "sgb.h"


void SGB::start_transfer_mode() {
    sgb_transfer_mode = true;
}

void SGB::write(const u8 a_value) {
    const u8 value = a_value & 0x30;
    const u8 previous = latch_value;
    latch_value = value;

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
        case PAL01:
            std::println("PAL01");
            palettes[0, 0] = packets[0][1] | (packets[0][2] << 8);
            palettes[0, 1] = packets[0][3] | (packets[0][4] << 8);
            palettes[0, 2] = packets[0][5] | (packets[0][6] << 8);
            palettes[0, 3] = packets[0][7] | (packets[0][8] << 8);

            palettes[1, 0] = palettes[0, 0];
            palettes[1, 1] = packets[0][9] | (packets[0][10] << 8);
            palettes[1, 2] = packets[0][11] | (packets[0][12] << 8);
            palettes[1, 3] = packets[0][13] | (packets[0][14] << 8);
            break;
        case PAL23:
            std::println("PAL23");
            palettes[2, 0] = packets[0][1] | (packets[0][2] << 8);
            palettes[2, 1] = packets[0][3] | (packets[0][4] << 8);
            palettes[2, 2] = packets[0][5] | (packets[0][6] << 8);
            palettes[2, 3] = packets[0][7] | (packets[0][8] << 8);

            palettes[3, 0] = palettes[0, 0];
            palettes[3, 1] = packets[0][9] | (packets[0][10] << 8);
            palettes[3, 2] = packets[0][11] | (packets[0][12] << 8);
            palettes[3, 3] = packets[0][13] | (packets[0][14] << 8);
            break;
        case PAL03:
            std::println("PAL03");
            palettes[0, 0] = packets[0][1] | (packets[0][2] << 8);
            palettes[0, 1] = packets[0][3] | (packets[0][4] << 8);
            palettes[0, 2] = packets[0][5] | (packets[0][6] << 8);
            palettes[0, 3] = packets[0][7] | (packets[0][8] << 8);

            palettes[3, 0] = palettes[0, 0];
            palettes[3, 1] = packets[0][9] | (packets[0][10] << 8);
            palettes[3, 2] = packets[0][11] | (packets[0][12] << 8);
            palettes[3, 3] = packets[0][13] | (packets[0][14] << 8);
            break;
        case PAL12:
            std::println("PAL12");
            palettes[1, 0] = packets[0][1] | (packets[0][2] << 8);
            palettes[1, 1] = packets[0][9] | (packets[0][10] << 8);
            palettes[1, 2] = packets[0][11] | (packets[0][12] << 8);
            palettes[1, 3] = packets[0][13] | (packets[0][14] << 8);

            palettes[2, 0] = packets[0][1] | (packets[0][2] << 8);
            palettes[2, 1] = packets[0][3] | (packets[0][4] << 8);
            palettes[2, 2] = packets[0][5] | (packets[0][6] << 8);
            palettes[2, 3] = packets[0][7] | (packets[0][8] << 8);
            break;
        case PAL_SET:
            std::println("PAL_SET");
            for (size_t palette_number = 0; palette_number < 4; ++palette_number) {
                const auto palette_id = packets[0][palette_number * 2 + 1] | (packets[0][palette_number * 2 + 2] << 8);
                for (size_t i = 0; i < 4; ++i) {
                    palettes[palette_number, i] = system_palettes[palette_id, i];
                }
            }
            break;
        case PAL_TRN:
            std::println("PAL_TRN");
            emit_command();
            break;
        case DATA_SND:
            std::println("DATA_SND command");
            std::println("Writing into SNES address {:x}", packets[0][1] | (packets[0][2] << 8));
            break;
        case DATA_TRN:
            std::println("DATA_TRN command");
            std::println("VRAM transfer into SNES address {:x}", packets[0][1] | (packets[0][2] << 8));
            break;
        case MLT_REQ: {
            std::println("MLT_REQ command");
            mlt_req.changing_joyp = false;
            switch (packets[0][1] & 3) {
                case 0:
                    mlt_req.current_player = 1;
                    mlt_req.players = 1;
                    std::println("MLT_REQ: Player 1");
                    break;
                case 1:
                    mlt_req.current_player = (mlt_req.current_player - 1) & 1;
                    mlt_req.players = 2;
                    std::println("MLT_REQ: Player 2");
                    break;
                case 3:
                    mlt_req.current_player = (mlt_req.current_player - 1) & 3;
                    mlt_req.players = 4;
                    std::println("MLT_REQ: Player 4");
                    break;
                default:
                    mlt_req.current_player = (mlt_req.current_player - 1) & 1;
                    mlt_req.players = 1;
                    std::println("MLT_REQ: Player default");
                    break;
            }
        }
        break;
        case CHR_TRN:
            std::println("CHR_TRN command");
            emit_command();
            break;
        case PCT_TRN:
            std::println("PCT_TRN command");
            emit_command();
            break;
        case ATTR_TRN:
            std::println("ATTR_TRN command (Unhandled");
            break;
        case MASK_EN:
            std::println("MASK_EN command");
            emit_command();
            break;
        default:
            std::println("Unhandled SGB command: {:x}", command);
            break;
    }
}

[[nodiscard]] bool SGB::is_sgb_transfer_mode() const {
    return sgb_transfer_mode;
}

bool SGB::mlt_is_active() const { return mlt_req.changing_joyp; }

u8 SGB::mlt_get_current_player() const {
    switch (mlt_req.current_player) {
        case 1: return 0xF;
        case 2: return 0xE;
        case 3: return 0xD;
        case 4: return 0xC;
        default: return 0xF;
    }
}

void SGB::mlt_change_joyp(const bool value) {
    mlt_req.changing_joyp = value;
}

void SGB::write_sgb_system_palette(const size_t index, const u16 data) {
    system_palettes_data[index] = data;
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

void SGB::write_sgb_tile_data() {
    const auto tile_transfer_info = packets[0][1];
    if ((tile_transfer_info & 1) == 0) {
        for (size_t i = 0; i < 0x1000; ++i) {
            tile_data[i] = vram_transfer_buffer[i];
        }
    } else {
        for (size_t i = 0; i < 0x1000; ++i) {
            tile_data[0x1000 + i] = vram_transfer_buffer[i];
        }
    }
}

void SGB::write_sgb_system_palette() {
    /*
     * There are 512 system palettes
     * Each palette has 4 colors
     * Each color is 2 bytes
     * So, each palette is 8 bytes
     */
    for (size_t palette = 0; palette < 512; ++palette) {
        for (size_t index = 0; index < 8; ++index) {
            const auto buffer_index = 2 * (palette * 8 + index);
            const u16 data = vram_transfer_buffer[buffer_index] | (vram_transfer_buffer[buffer_index + 1] << 8);
            system_palettes_data[buffer_index / 2] = data;
        }
    }
}

void SGB::emit_command() const {
    for (const auto &func: listeners) {
        func(command, packets);
    }
}

std::span<u32> SGB::get_sgb_video_buffer() {
    /*
     * The plan
     *
     * First, go entry by entry in the map to get the data
     * Then get pattern
     * Get the palette and colors.
     *
     * Write it to the buffer;
     */

    std::mdspan buffer_view{buffer.data(), std::extents<size_t, 224, 256>{}};

    for (size_t y = 0; y < 28; ++y) {
        for (size_t x = 0; x < 32; ++x) {
            const auto tile_map_index = (y * 32 + x) * 2;
            const auto tile_map_entry = tile_map[tile_map_index] | (tile_map[tile_map_index + 1] << 8);

            const auto tile_index = tile_map_entry & 0xFF;
            const auto palette_index = (tile_map_entry >> 10) & 0x7;

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
                u8 byte0 = tile_data[tile_index * 32 + row * 2];
                u8 byte1 = tile_data[tile_index * 32 + row * 2 + 1];
                u8 byte2 = tile_data[tile_index * 32 + row * 2 + 16];
                u8 byte3 = tile_data[tile_index * 32 + row * 2 + 17];

                for (size_t small_x = 0; small_x < 8; ++small_x) {
                    const auto color_index = ((byte0 >> (7 - small_x)) & 1) |
                                             (((byte1 >> (7 - small_x)) & 1) << 1) |
                                             (((byte2 >> (7 - small_x)) & 1) << 2) |
                                             (((byte3 >> (7 - small_x)) & 1) << 3);
                    // std::println("Color index: {}", color_index);

                    const auto color = palettes[palette_index, color_index];

                    const u8 red = color & 0x1F;
                    const u8 green = (color >> 5) & 0x1F;
                    const u8 blue = (color >> 10) & 0x1F;
                    // std::println("Red: {:x}, Green: {:x}, Blue: {:x}", red, green, blue);

                    const u8 red8 = (red << 3) | (red >> 2);
                    const u8 green8 = (green << 3) | (green >> 2);
                    const u8 blue8 = (blue << 3) | (blue >> 2);
                    buffer_view[y * 8 + row, x * 8 + small_x] = (red8 << 16) | (green8 << 8) | blue8;
                }
            }
        }
    }

    return buffer;
}

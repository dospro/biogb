#include "sgb.h"


void SGB::start_transfer_mode() {
    sgb_transfer_mode = true;
}

void SGB::send_bit(const uint8_t bit) {
    packer.add_bit(bit);
    if (packer.is_full()) {
        sgb_transfer_mode = false;
        if (length == 0) {
            const auto value = packer.get_byte(0);
            length = value & 7;
            command = value >> 3;
        }
        packets.emplace_back(packer.get_packet());
        packer.reset();

        if (packets.size() == length) {
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

                    break;
                case PAL_TRN:
                    std::println("PAL_TRN");
                    emit_command();
                    break;

                case MLT_REQ:
                    std::println("MLT_REQ command");
                    // emit_command();
                    mlt_req = MultiPlayer{
                        .changing_joyp = false,
                        .players = packets[0][0],
                        .current_player = 0,
                    };
                    break;
                case MASK_EN:
                    std::println("MASK_EN command");
                    emit_command();
                default:
                    std::println("Unhandled SGB command: {:x}", command);
                    break;
            }
            packets.clear();
            length = 0;
            command = 0;
        }
    }
}

[[nodiscard]] bool SGB::is_sgb_transfer_mode() const {
    return sgb_transfer_mode;
}

bool SGB::mlt_is_active() const { return mlt_req.changing_joyp; }

u8 SGB::mlt_get_current_player() const {
    std::println("Reading MLT");
    switch (mlt_req.current_player) {
        case 1: return 0xF;
        case 2: return 0xE;
        case 3: return 0xD;
        case 4: return 0xC;
        default: return 0xF;
    }
}

void SGB::mlt_change_joyp(bool value) {
    mlt_req.changing_joyp = value;
}

void SGB::write_sgb_palette(const size_t index, const u16 data) {
    system_palettes_data[index] = data;
}

void SGB::emit_command() const {
    for (const auto& func : listeners) {
        func(command, packets);
    }
}

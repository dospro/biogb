#include "sound.h"

#include <algorithm>

cSound::cSound(const int a_generalFrequency) :
    channel_1{a_generalFrequency},
    channel_2{a_generalFrequency},
    channel_3{a_generalFrequency},
    channel_4{a_generalFrequency},
    GENERAL_FREQUENCY{a_generalFrequency},
    base_cycles_per_sample{cpu_clock_hz / a_generalFrequency} {
    update_cycles_per_sample();
}

u8 cSound::readFromSound(const u16 a_address) const {
    if (a_address < 0xFF16)
        return channel_1.readRegister(a_address);
    else if (a_address < 0xFF1A)
        return channel_2.readRegister(a_address);
    else if (a_address < 0xFF20)
        return channel_3.readRegister(a_address);
    else if (a_address < 0xFF24)
        return channel_4.readRegister(a_address);
    else if (a_address < 0xFF30) {
        switch (a_address) {
            case 0xFF24: // NR 50
                return NR50;
            case 0xFF25: // NR 51
                return NR51;
            case 0xFF26: // NR 52
                return static_cast<u8>(channel_1.getOnOffBit()
                                       | channel_2.getOnOffBit()
                                       | channel_3.getOnOffBit()
                                       | channel_4.getOnOffBit()
                                       | (soundActive << 7)
                                       | 0x70);
            default:
                return 0xFF;
        }
    } else
        return channel_3.readRegister(a_address);
}

void cSound::writeToSound(const u16 address, const u8 value) {
    switch (address) {
        case 0xFF10: //NR 10
        case 0xFF11: //NR 11
        case 0xFF12: //NR 12
        case 0xFF13: //NR 13
        case 0xFF14: //NR 14
            channel_1.writeRegister(address, value);
            break;
        case 0xFF16: //NR 21
        case 0xFF17: //NR 22
        case 0xFF18: //NR 23
        case 0xFF19: //NR 24
            channel_2.writeRegister(address, value);
            break;
        case 0xFF1A: //NR 30
        case 0xFF1B: //NR 31
        case 0xFF1C: //NR 32
        case 0xFF1D: //NR 33
        case 0xFF1E: //NR 34
            channel_3.writeRegister(address, value);
            break;
        case 0xFF20: //NR 41
        case 0xFF21: //NR 42
        case 0xFF22: //NR 43
        case 0xFF23: //NR 44
            channel_4.writeRegister(address, value);
            break;
        case 0xFF24: //NR 50
            NR50 = value;
            break;
        case 0xFF25:
            channel_1.setOutputTerminal((value & 1) != 0, (value & 0x10) != 0);
            channel_2.setOutputTerminal((value & 2) != 0, (value & 0x20) != 0);
            channel_3.setOutputTerminal((value & 4) != 0, (value & 0x40) != 0);
            channel_4.setOutputTerminal((value & 8) != 0, (value & 0x80) != 0);
            NR51 = value;
            break;
        case 0xFF26: //NR 52
            soundActive = ((value & 0x80) != 0);
            break;
        default:
            if (address >= 0xFF30 && address < 0xFF40) {
                channel_3.writeRegister(address, value);
            }
            break;
    }
}

void cSound::updateCycles(const s32 cycles) {
    channel_1.update(cycles);
    channel_2.update(cycles);
    channel_3.update(cycles);
    channel_4.update(cycles);
    cycles_counter += static_cast<double>(cycles);
    while (cycles_counter >= cycles_per_sample) {
        const int left_master_volume = (NR50 >> 4) & 7;
        const int right_master_volume = NR50 & 0x7;

        cycles_counter -= cycles_per_sample;

        int sample1 = 0, sample2 = 0, sample3 = 0, sample4 = 0;

        if (soundActive) {
            if (channel_1.isOn()) sample1 = channel_1.getSample() - 128;
            if (channel_2.isOn()) sample2 = channel_2.getSample() - 128;
            if (channel_3.isOn()) sample3 = channel_3.getSample() - 128;
            if (channel_4.isOn()) sample4 = channel_4.getSample() - 128;
        }
        int left_final_sample = 0;
        if (channel_1.isLeftSound()) left_final_sample += sample1;
        if (channel_2.isLeftSound()) left_final_sample += sample2;
        if (channel_3.isLeftSound()) left_final_sample += sample3;
        if (channel_4.isLeftSound()) left_final_sample += sample4;

        left_final_sample >>= 2;
        left_final_sample = (left_final_sample * left_master_volume) / 7;
        left_final_sample = std::clamp(left_final_sample, -128, 127);

        int right_final_sample = 0;
        if (channel_1.isRightSound()) right_final_sample += sample1;
        if (channel_2.isRightSound()) right_final_sample += sample2;
        if (channel_3.isRightSound()) right_final_sample += sample3;
        if (channel_4.isRightSound()) right_final_sample += sample4;

        right_final_sample >>= 2;
        right_final_sample = (right_final_sample * right_master_volume) / 7;
        right_final_sample = std::clamp(right_final_sample, -128, 127);

        buffer.push_back(static_cast<u8>(left_final_sample + 128));
        buffer.push_back(static_cast<u8>(right_final_sample + 128));
    }
}

void cSound::adjust_sample_rate(const std::size_t queued_samples) {
    const auto error = (static_cast<double>(queued_samples) - queue_target_samples) / queue_target_samples;
    adjust_factor = 1.0 + std::clamp(error * drc_gain, -drc_max_delta, drc_max_delta);
    update_cycles_per_sample();
}

void cSound::set_speed(const double new_speed) {
    speed = std::clamp(new_speed, min_speed, max_speed);
    update_cycles_per_sample();
}

void cSound::update_cycles_per_sample() {
    cycles_per_sample = base_cycles_per_sample * speed * adjust_factor;
}

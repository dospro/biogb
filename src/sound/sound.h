#ifndef BIOGB_SOUND
#define BIOGB_SOUND

#include <cstddef>
#include <span>
#include <vector>

#include "../macros.h"
#include "sound_channel_1.h"
#include "sound_channel_2.h"
#include "sound_channel_3.h"
#include "sound_channel_4.h"

class cSound {
public:
    explicit cSound(int a_generalFrequency);
    ~cSound() = default;
    [[nodiscard]] u8 readFromSound(u16 a_address) const;
    void writeToSound(u16 address, u8 value);
    void updateCycles(s32 cycles);
    [[nodiscard]] std::span<const u8> get_samples() const { return buffer; }
    void clear_samples() { buffer.clear(); }
    void adjust_sample_rate(std::size_t queued_samples);
    void set_speed(double new_speed);

private:
    cSoundChannel1 channel_1;
    cSoundChannel2 channel_2;
    cSoundChannel3 channel_3;
    cSoundChannel4 channel_4;

    std::vector<u8> buffer;
    double cycles_counter{};

    static constexpr double cpu_clock_hz = 4194304.0;
    static constexpr double queue_target_samples = 2000.0;
    static constexpr double drc_gain = 0.05;
    static constexpr double drc_max_delta = 0.005;
    static constexpr double min_speed = 0.25;
    static constexpr double max_speed = 64.0;

    const int GENERAL_FREQUENCY{};
    const double base_cycles_per_sample{};
    double cycles_per_sample{};
    static constexpr int NUMBER_OF_CHANNELS{2};
    bool soundActive{true};
    double speed{1.0};
    double adjust_factor{1.0};
    u8 NR50{};
    u8 NR51{};

    void update_cycles_per_sample();
};

#endif
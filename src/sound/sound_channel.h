#ifndef BIOGB_SOUND_CHANNEL_H
#define BIOGB_SOUND_CHANNEL_H

#include <array>

class cSoundChannel {
public:
    explicit cSoundChannel(int a_generalFrequency);
    ~cSoundChannel() = default;
    void setOutputTerminal(bool a_left, bool a_right);
    [[nodiscard]] bool isLeftSound() const noexcept;
    [[nodiscard]] bool isRightSound() const noexcept;
    [[nodiscard]] bool isOn() const noexcept;

protected:
    const int CYCLES_PER_SECOND = 4194304;
    const int GENERAL_FREQUENCY{};
    static constexpr std::array<std::array<int, 8>, 4> mWavePatternDuty{
        {
            {{0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}, // 12.5% duty
            {{0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}, // 25% duty
            {{0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF}}, // 50% duty
            {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF}} // 75% duty
        }
    };
    bool mOnOff{};
    bool outputTerminal1{}, outputTerminal2{};
};

#endif  // BIOGB_SOUND_CHANNEL_H

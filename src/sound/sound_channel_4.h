#ifndef BIOGB_SOUND_CHANNEL_4_H
#define BIOGB_SOUND_CHANNEL_4_H

#include "sound_channel.h"

class cSoundChannel4 : public cSoundChannel
{

public:
    explicit cSoundChannel4(int a_generalFrequency);
    ~cSoundChannel4() = default;
    int readRegister(int a_address) const;
    void writeRegister(int a_address, int a_value);
    int getSample();
    void update(int a_cycles);
    int getOnOffBit() const;

private:
    void writeNR41(int a_value);
    void writeNR42(int a_value);
    void writeNR43(int a_value);
    void writeNR44(int a_value);
    void setEnvelopTimer(int a_value);
    std::array<int, 0x8000> mRandomWave{};
    int NR41{};
    int NR42{};
    int NR43{};
    int NR44{};
    int mSoundLength{};
    int mInitialVolumen{};
    bool mVolumenIncrease{};
    int mEnvelopSweep{};
    int mFrequency{};
    double mPeriod{};
    bool mConsecutive{};
    double mCounter{};
    int mShiftClock{};
    int mPatternWidth{};
    int mDivisorCode{};
    int mShiftRegister{0x7FFF};
    int mPatternIndex{};
};


#endif  // BIOGB_SOUND_CHANNEL_4_H

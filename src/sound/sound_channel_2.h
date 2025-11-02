#ifndef BIOGB_SOUND_CHANNEL_2_H
#define BIOGB_SOUND_CHANNEL_2_H

#include "sound_channel.h"

class cSoundChannel2 : public cSoundChannel
{

public:
    explicit cSoundChannel2(int a_generalFrequency);
    ~cSoundChannel2() = default;
    int readRegister(int a_address) const;
    void writeRegister(int a_address, int a_value);
    int getSample();
    void update(int a_cycles);
    int getOnOffBit() const;
private:
    void writeNR21(int a_value);
    void writeNR22(int a_value);
    void writeNR23(int a_value);
    void writeNR24(int a_value);
    void setEnvelopTimer(int a_value);
    void setFrequency();
    int NR21{};
    int NR22{};
    int NR23{};
    int NR24{};
    int mInitialVolumen{};
    bool mIncreaseVolumen{};
    int mEnvelopSweep{};
    int mWavePatternType{};
    int mSoundLength{};
    int mFrequency{};
    double mSamplesPerSecond{};
    bool mConsecutive{};
    double mCounter{};
    int mPatternIndex{};

};


#endif  // BIOGB_SOUND_CHANNEL_2_H

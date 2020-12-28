//
// Created by dospro on 14/12/15.
//

#ifndef BIOGB_SOUND_CHANNEL_3_H
#define BIOGB_SOUND_CHANNEL_3_H

#include "sound_channel.h"

class cSoundChannel3 : public cSoundChannel
{
public:
    cSoundChannel3(int a_generalFrecuency);
    virtual ~cSoundChannel3();
    virtual int readRegister(int a_address) override;
    virtual void writeRegister(int a_address, int a_value) override;
    virtual int getSample() override;
    virtual void update(int a_cycles) override;
    virtual int getOnOffBit() override;
private:
    void writeNR30(int a_value);
    void writeNR31(int a_value);
    void writeNR32(int a_value);
    void writeNR33(int a_value);
    void writeNR34(int a_value);
    void writeWaveRam(int a_address, int a_value);
    void setFrequency();
    std::array<int, 32> mWaveRam;
    int NR30;
    int NR31;
    int NR32;
    int NR33;
    int NR34;
    int mWaveLastWrittenValue;
    int mSoundLength;
    int mOutputLevel;
    int mFrequency;
    double mSamplePerSecond;
    bool mConsecutive;
    bool mDACBit;
    double mCounter;
    int mPatternIndex;
};


#endif  // BIOGB_SOUND_CHANNEL_3_H

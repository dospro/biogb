//
// Created by dospro on 17/12/15.
//

#ifndef BIOGB_CSOUNDCHANNEL2_H
#define BIOGB_CSOUNDCHANNEL2_H


#include "cSoundChannel.h"

class cSoundChannel2 : public cSoundChannel
{

public:
    cSoundChannel2(int a_generalFrecuency);
    virtual ~cSoundChannel2();
    virtual int readRegister(int a_address) override;
    virtual void writeRegister(int a_address, int a_value) override;
    virtual int getSample() override;
    virtual void update(int a_cycles) override;
    virtual int getOnOffBit() override;
private:
    void writeNR21(int a_value);
    void writeNR22(int a_value);
    void writeNR23(int a_value);
    void writeNR24(int a_value);
    void setEnvelopTimer(int a_value);
    void setFrequency();
    int NR21;
    int NR22;
    int NR23;
    int NR24;
    int mInitialVolumen;
    bool mIncreaseVolumen;
    int mEnvelopSweep;
    int mWavePatternType;
    int mSoundLength;
    int mFrequency;
    double mSamplesPerSecond;
    bool mConsecutive;
    double mCounter;
    int mPatternIndex;

};


#endif //BIOGB_CSOUNDCHANNEL2_H

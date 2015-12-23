//
// Created by dospro on 14/12/15.
//

#ifndef BIOGB_CSOUNDCHANNEL1_H
#define BIOGB_CSOUNDCHANNEL1_H


#include "cSoundChannel.h"

class cSoundChannel1 : public cSoundChannel
{

public:
    cSoundChannel1(int a_generalFrecuency);
    virtual ~cSoundChannel1();
    virtual int readRegister(int a_address) override;
    virtual void writeRegister(int a_address, int a_value) override;
    virtual int getSample() override;
    virtual void update(int a_cycles) override;
    virtual int getOnOffBit() override;

private:
    void writeNR10(int a_value);
    void writeNR11(int a_value);
    void writeNR12(int a_value);
    void writeNR13(int a_value);
    void writeNR14(int a_value);
    void sweepTrigger();
    void setEnvelopTimer(int a_value);
    void calculateNewFrequency();
    void setSweepTimer(int a_value);
    void setLengthTimer(int a_value);
    void setFrequency();
    int NR10;
    int NR11;
    int NR12;
    int NR13;
    int NR14;
    int mSweepTime;
    bool mSweepIncrement;
    int mSweepShift;
    int mWavePatternType;
    int mSoundLength;
    int mInitialVolumen;
    bool mVolumenIncrease;
    int mEnvelopSweep;
    int mFrequency;
    double mSamplesPerSecond;
    bool mConsecutive;
    int mShadowFrequency;
    double mCounter;
    int mPatternIndex;
    bool mSweepEnabled;

};


#endif //BIOGB_CSOUNDCHANNEL1_H

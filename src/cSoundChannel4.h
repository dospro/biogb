//
// Created by dospro on 18/12/15.
//

#ifndef BIOGB_CSOUNDCHANNEL4_H
#define BIOGB_CSOUNDCHANNEL4_H


#include "cSoundChannel.h"

class cSoundChannel4 : public cSoundChannel
{

public:
    cSoundChannel4(int a_generalFrecuency);
    virtual ~cSoundChannel4();
    virtual int readRegister(int a_address) override;
    virtual void writeRegister(int a_address, int a_value) override;
    virtual int getSample() override;
    virtual void update(int a_cycles) override;
    virtual int getOnOffBit() override;

private:
    void writeNR41(int a_value);
    void writeNR42(int a_value);
    void writeNR43(int a_value);
    void writeNR44(int a_value);
    void setEnvelopTimer(int a_value);
    std::array<int, 0x8000> mRandomWave;
    int NR41;
    int NR42;
    int NR43;
    int NR44;
    int mSoundLength;
    int mInitialVolumen;
    bool mVolumenIncrease;
    int mEnvelopSweep;
    int mFrequency;
    double mPeriod;
    bool mConsecutive;
    double mCounter;
    int mShiftClock;
    int mPatternWidth;
    int mDivisorCode;
    int mShiftRegister;
    int mPatternIndex;
};


#endif //BIOGB_CSOUNDCHANNEL4_H

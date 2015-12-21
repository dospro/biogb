//
// Created by dospro on 17/12/15.
//

#include "cSoundChannel2.h"

cSoundChannel2::cSoundChannel2(int a_generalFrecuency) :
        cSoundChannel(a_generalFrecuency),
        mPatternIndex{0},
        mCounter{0}
{

}

cSoundChannel2::~cSoundChannel2()
{

}

int cSoundChannel2::readRegister(int a_address)
{
    switch (a_address)
    {
        case 0xFF16:
            return NR21;
        case 0xFF17:
            return NR22;
        case 0xFF18:
            return NR23;
        case 0xFF19:
            return NR24;
        default:
            return 0xFF;
    }
}

void cSoundChannel2::writeRegister(int a_address, int a_value)
{
    switch (a_address)
    {
        case 0xFF16:
            writeNR21(a_value);
            break;
        case 0xFF17:
            writeNR22(a_value);
            break;
        case 0xFF18:
            writeNR23(a_value);
            break;
        case 0xFF19:
            writeNR24(a_value);
            break;
        default:
            break;
    }
}

int cSoundChannel2::getOnOffBit()
{
    if (mOnOff)
        return 2;
    return 0;
}

void cSoundChannel2::writeNR21(int a_value)
{
    mWavePatternType = a_value >> 6;
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
    NR21 = a_value | 0x3F;
}

void cSoundChannel2::writeNR22(int a_value)
{
    mInitialVolumen = a_value >> 4;
    mIncreaseVolumen = ((a_value & 8) != 0);
    mEnvelopSweep = (a_value & 7) * (CYCLES_PER_SECOND / 64);
    NR22 = a_value;
}

void cSoundChannel2::writeNR23(int a_value)
{
    mFrequency = (mFrequency & 0x700) | a_value;
    NR23 = 0xFF;
}

void cSoundChannel2::writeNR24(int a_value)
{
    mFrequency = (mFrequency & 0xFF) | ((a_value & 7) << 8);
    mConsecutive = ((a_value & 0x40) == 0);
    if ((a_value & 0x80) != 0) // Trigger
    {
        mOnOff = true;
        mSoundLength = (mSoundLength == 0) ? 64 * (CYCLES_PER_SECOND / 256) : mSoundLength;
        mInitialVolumen = NR22 >> 4;
        if (mInitialVolumen == 0)
            mOnOff = false;
    }
    NR24 = a_value | 0xBF;
}

int cSoundChannel2::getSample()
{
    double finalFrequency = 131072 / (2048 - mFrequency);
    double samplesPerSecond = GENERAL_FREQUENCY / finalFrequency / mWavePatternDuty[mWavePatternType].size();
    int sample{0};

    if (mCounter <= samplesPerSecond)
    {
        ++mCounter;
        sample = mWavePatternDuty[mWavePatternType][mPatternIndex];
    }
    else
    {
        ++mPatternIndex;
        if (mPatternIndex >= mWavePatternDuty[mWavePatternType].size())
            mPatternIndex = 0;
        mCounter -= samplesPerSecond - 1;
        sample = mWavePatternDuty[mWavePatternType][mPatternIndex];
    }
    return (mInitialVolumen * sample) / 15;
}

void cSoundChannel2::update(int a_cycles)
{
    if (!mConsecutive)
    {
        mSoundLength -= a_cycles;
        if (mSoundLength <= 0)
        {
            mOnOff = false;
            mSoundLength = 0;
        }
    }
    if (mEnvelopSweep != 0 && mOnOff)
    {
        mEnvelopSweep -= a_cycles;
        if (mEnvelopSweep <= 0)
        {
            if (mIncreaseVolumen)
            {
                mInitialVolumen++;
                if (mInitialVolumen > 0xF)
                    mInitialVolumen = 0xF;
            }
            else
            {
                mInitialVolumen--;
                if (mInitialVolumen < 0)
                    mInitialVolumen = 0;
            }
            mEnvelopSweep = (NR22 & 7) * (CYCLES_PER_SECOND / 64);
        }
    }
}

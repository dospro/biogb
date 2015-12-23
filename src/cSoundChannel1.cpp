//
// Created by dospro on 14/12/15.
//

#include "cSoundChannel1.h"

cSoundChannel1::cSoundChannel1(int a_generalFrecuency) :
        cSoundChannel(a_generalFrecuency),
        mPatternIndex{0},
        mCounter{0}
{
}

cSoundChannel1::~cSoundChannel1()
{
}

int cSoundChannel1::readRegister(int a_address)
{
    switch (a_address)
    {
        case 0xFF10:
            return NR10 | 0x80;
        case 0xFF11:
            return NR11 | 0x3F;
        case 0xFF12:
            return NR12;
        case 0xFF13:
            return NR13 | 0xFF;
        case 0xFF14:
            return NR14 | 0xBF;
        default:
            return 0xFF;
    }
}

void cSoundChannel1::writeRegister(int a_address, int a_value)
{
    switch (a_address)
    {
        case 0xFF10:
            writeNR10(a_value);
            break;
        case 0xFF11:
            writeNR11(a_value);
            break;
        case 0xFF12:
            writeNR12(a_value);
            break;
        case 0xFF13:
            writeNR13(a_value);
            break;
        case 0xFF14:
            writeNR14(a_value);
            break;
        default:
            break;
    }
}

int cSoundChannel1::getOnOffBit()
{
    if (mOnOff)
        return 1;
    return 0;
}

int cSoundChannel1::getSample()
{
    if (mCounter <= mSamplesPerSecond)
        ++mCounter;
    else
    {
        ++mPatternIndex;
        if (mPatternIndex >= mWavePatternDuty[mWavePatternType].size())
            mPatternIndex = 0;
        mCounter -= mSamplesPerSecond - 1;
    }
    return (mInitialVolumen * mWavePatternDuty[mWavePatternType][mPatternIndex]) / 15;

}

void cSoundChannel1::update(int a_cycles)
{
    if (mOnOff && !mConsecutive)
    {
        mSoundLength -= a_cycles;
        if (mSoundLength <= 0)
        {
            mOnOff = false;
            mSoundLength = 0;
        }
    }
    if (mOnOff && mEnvelopSweep != 0)
    {
        mEnvelopSweep -= a_cycles;
        if (mEnvelopSweep <= 0)
        {
            if (mVolumenIncrease)
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
            setEnvelopTimer(NR12);
        }
    }

    if (mOnOff && mSweepEnabled)
    {
        mSweepTime -= a_cycles;
        if (mSweepTime <= 0)
        {
            if (mSweepIncrement)
            {
                mShadowFrequency += mShadowFrequency >> (mSweepShift + 1);
                if (mShadowFrequency > 2047)
                {
                    mShadowFrequency = 2047;
                    mOnOff = false;
                }
                else
                {
                    if (mSweepShift != 0)
                    {
                        NR13 = mShadowFrequency & 0xFF;
                        NR14 = (NR14 & 0xF8) | ((mShadowFrequency >> 8) & 7);
                        mFrequency = mShadowFrequency;
                        setFrequency();
                        calculateNewFrequency();
                    }
                    setSweepTimer(NR10);
                }
            }
            else
            {
                mShadowFrequency -= mShadowFrequency >> (mSweepShift + 1);
                if (mShadowFrequency < 0)
                    mShadowFrequency = 0;
                else
                {
                    if (mSweepShift != 0)
                    {
                        NR13 = mShadowFrequency & 0xFF;
                        NR14 = (NR14 & 0xF8) | ((mShadowFrequency >> 8) & 7);
                        mFrequency = mShadowFrequency;
                        setFrequency();
                        calculateNewFrequency();
                    }
                    setSweepTimer(NR10);
                }

            }
        }
    }
}

void cSoundChannel1::writeNR10(int a_value)
{
    setSweepTimer(a_value);
    mSweepIncrement = ((a_value >> 3) & 1) == 0;
    mSweepShift = a_value & 7;
    mSweepEnabled = false;
    NR10 = a_value;
}

void cSoundChannel1::writeNR11(int a_value)
{
    mWavePatternType = a_value >> 6;
    setLengthTimer(a_value);
    NR11 = a_value;
}

void cSoundChannel1::writeNR12(int a_value)
{
    mInitialVolumen = a_value >> 4;
    mVolumenIncrease = ((a_value >> 3) & 1) != 0;
    setEnvelopTimer(a_value);
    NR12 = a_value;
}

void cSoundChannel1::writeNR13(int a_value)
{
    mFrequency = (mFrequency & 0x700) | a_value;
    setFrequency();
    NR13 = a_value;
}

void cSoundChannel1::writeNR14(int a_value)
{
    mFrequency = (mFrequency & 0xFF) | ((a_value & 7) << 8);
    setFrequency();
    mConsecutive = ((a_value >> 6) & 1) == 0;
    if ((a_value >> 7) == 1) // Trigger
    {
        mOnOff = true;
        mSoundLength = (mSoundLength == 0) ? 64 * (CYCLES_PER_SECOND / 256) : mSoundLength;
        mCounter = 0;
        mPatternIndex = 0;
        setEnvelopTimer(NR12);
        mInitialVolumen = NR12 >> 4;
        if ((NR12 & 0xF8) == 0)
            mOnOff = false;

        sweepTrigger();

    }
    NR14 = a_value;
}

void cSoundChannel1::sweepTrigger()
{
    mShadowFrequency = mFrequency;
    setSweepTimer(NR10);
    mSweepEnabled = true;
    if (mSweepTime == 0 && mSweepShift == 0)
        mSweepEnabled = false;

    if (mSweepShift != 0)
    {
        calculateNewFrequency();
    }
}

void cSoundChannel1::calculateNewFrequency()
{
    int freqIncrement = mShadowFrequency >> (mSweepShift + 1);
    mShadowFrequency += (mSweepIncrement ? freqIncrement : -freqIncrement);
    if (mShadowFrequency > 2047)
    {
        mShadowFrequency = 2047;
        mOnOff = false;
    }
    else if (mShadowFrequency <= 0)
        mShadowFrequency = 0;
}

void cSoundChannel1::setSweepTimer(int a_value)
{
    mSweepTime = ((a_value & 0x70) >> 4) * (CYCLES_PER_SECOND / 128);
}

void cSoundChannel1::setLengthTimer(int a_value)
{
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
}

void cSoundChannel1::setEnvelopTimer(int a_value)
{
    mEnvelopSweep = a_value & 7;
    //if (mEnvelopSweep == 0)
    //    mEnvelopSweep = 8;
    mEnvelopSweep *= (CYCLES_PER_SECOND / 64);
}

void cSoundChannel1::setFrequency()
{
    double finalFrequency = 131072 / (2048 - mFrequency);
    mSamplesPerSecond = GENERAL_FREQUENCY / finalFrequency / mWavePatternDuty[mWavePatternType].size();
}
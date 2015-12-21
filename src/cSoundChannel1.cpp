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
            return NR10;
        case 0xFF11:
            return NR11;
        case 0xFF12:
            return NR12;
        case 0xFF13:
            return NR13;
        case 0xFF14:
            return NR14;
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
    double finalFrequency = 131072 / (2048 - mShadowFrequency);
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

void cSoundChannel1::update(int a_cycles)
{
    if (!mConsecutive && mSoundLength != 0)
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
            mEnvelopSweep = (NR12 & 7) * (CYCLES_PER_SECOND / 64);
        }
    }

    if (mSweepTime != 0 && mOnOff)
    {
        mSweepTime -= a_cycles;
        if (mSweepTime <= 0)
        {
            if (mSweepIncrement)
            {
                mShadowFrequency += mShadowFrequency >> mSweepShift;
                if (mShadowFrequency > 2047)
                {
                    mShadowFrequency = 2047;
                    mOnOff = false;
                }
                else
                {
                    mSweepTime = ((NR10 & 0x70) >> 4) * (CYCLES_PER_SECOND / 128);
                }
            }
            else
            {
                mShadowFrequency -= mShadowFrequency >> mSweepShift;
                if (mShadowFrequency < 0)
                    mShadowFrequency = 0;
                else
                    mSweepTime = ((NR10 & 0x70) >> 4) * (CYCLES_PER_SECOND / 128);

            }
        }
    }
}

void cSoundChannel1::writeNR10(int a_value)
{
    mSweepTime = ((a_value & 0x70) >> 4) * (CYCLES_PER_SECOND / 128);
    mSweepIncrement = ((a_value >> 3) & 1) == 0;
    mSweepShift = a_value & 7;
    NR10 = a_value | 0x80;
}

void cSoundChannel1::writeNR11(int a_value)
{
    mWavePatternType = a_value >> 6;
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
    NR11 = a_value | 0x3F;
}

void cSoundChannel1::writeNR12(int a_value)
{
    mInitialVolumen = a_value >> 4;
    mVolumenIncrease = ((a_value >> 3) & 1) != 0;
    mEnvelopSweep = (a_value & 7) * (CYCLES_PER_SECOND / 64);
    NR12 = a_value;
}

void cSoundChannel1::writeNR13(int a_value)
{
    mFrequency = (mFrequency & 0x700) | a_value;
    mShadowFrequency = mFrequency;
    NR13 = 0xFF;
}

void cSoundChannel1::writeNR14(int a_value)
{
    mFrequency = (mFrequency & 0xFF) | ((a_value & 7) << 8);
    mShadowFrequency = mFrequency;
    mConsecutive = ((a_value >> 6) & 1) == 0;
    if ((a_value >> 7) == 1) // Trigger
    {
        mOnOff = true;
        mShadowFrequency = mFrequency;
        mSoundLength = (mSoundLength == 0) ? 64 * (CYCLES_PER_SECOND / 256) : mSoundLength;
        mInitialVolumen = NR12 >> 4;
        if (mInitialVolumen == 0)
            mOnOff = false;

        mSweepTime = ((NR10 & 0x70) >> 4) * (CYCLES_PER_SECOND / 128);

        //  The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
        //if (mSweepTime == 0 || mSweepShift == 0)
        //    mOnOff = false;

        // If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
        if (mSweepShift != 0)
        {
            /*Frequency calculation consists of taking the value in the frequency shadow register,
             * shifting it right by sweep shift, optionally negating the value, and summing this
             * with the frequency shadow register to produce a new frequency. What is done with
             * this new frequency depends on the context. */
            int freqIncrement = mShadowFrequency >> mSweepShift;
            mShadowFrequency += (mSweepIncrement ? freqIncrement : -freqIncrement);
            if (mShadowFrequency > 2047)
            {
                mShadowFrequency = 2047;
                mOnOff = false;
            }
        }
    }
    NR14 = a_value | 0xBF;
}

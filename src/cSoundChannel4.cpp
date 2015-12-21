//
// Created by dospro on 18/12/15.
//

#include <iostream>
#include "cSoundChannel4.h"

cSoundChannel4::cSoundChannel4(int a_generalFrecuency) :
        cSoundChannel(a_generalFrecuency),
        mShiftRegister{0x7FFF},
        mPatternIndex{0},
        mCounter{0}
{
    for (int i = 0; i < mRandomWave.size(); ++i)
    {
        int xoredBit = ((mShiftRegister) ^ (mShiftRegister >> 1)) & 1;
        mShiftRegister >>= 1;
        mShiftRegister = mShiftRegister | (xoredBit << 14);
        int sample = (((mShiftRegister & 1) ^ 1) * 0xFF);
        mRandomWave[i] = sample;
    }
}

cSoundChannel4::~cSoundChannel4()
{

}

int cSoundChannel4::readRegister(int a_address)
{
    switch (a_address)
    {
        case 0xFF20:
            return NR41;
        case 0xFF21:
            return NR42;
        case 0xFF22:
            return NR43;
        case 0xFF23:
            return NR44;
        default:
            return 0xFF;
    }
}

void cSoundChannel4::writeRegister(int a_address, int a_value)
{
    switch (a_address)
    {
        case 0xFF20:
            writeNR41(a_value);
            break;
        case 0xFF21:
            writeNR42(a_value);
            break;
        case 0xFF22:
            writeNR43(a_value);
            break;
        case 0xFF23:
            writeNR44(a_value);
            break;
        default:
            break;
    }
}

int cSoundChannel4::getSample()
{
    if (mCounter <= mPeriod)
    {
        mCounter++;
    }
    else
    {
        ++mPatternIndex;
        if (mPatternIndex > mPatternWidth)
            mPatternIndex = 0;
        mCounter -= mPeriod;
    }
    return (mInitialVolumen * mRandomWave[mPatternIndex]) / 15;
    /*double frequency = mDivisorCode / mShiftClock;
    int size = (mPatternWidth ? 0x80 : 0x8000);
    double samplesPerSecond = 4194304 / frequency / size;

    if (mCounter <= samplesPerSecond)
    {
        ++mCounter;
    }
    else
    {
        ++mPatternIndex;
        if (mPatternIndex >= size)
            mPatternIndex = 0;
        mCounter -= samplesPerSecond - 1;
    }
    return (mInitialVolumen * mRandomWave[mPatternIndex]) / 15;*/
}

void cSoundChannel4::update(int a_cycles)
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
            mEnvelopSweep = (NR42 & 7) * (CYCLES_PER_SECOND / 64);
        }
    }
}

int cSoundChannel4::getOnOffBit()
{
    if (mOnOff)
        return 8;
    return 0;
}

void cSoundChannel4::writeNR41(int a_value)
{
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
    NR41 = 0xFF;
}

void cSoundChannel4::writeNR42(int a_value)
{
    mInitialVolumen = a_value >> 4;
    mVolumenIncrease = ((a_value >> 3) & 1) != 0;
    mEnvelopSweep = (a_value & 7) * (CYCLES_PER_SECOND / 64);
    NR42 = a_value;
}

void cSoundChannel4::writeNR43(int a_value)
{
    mShiftClock = (a_value >> 4);
    mPatternWidth = ((a_value & 8) != 0) ? 0x80 : 0x8000;
    mDivisorCode = a_value & 7;
    if (mDivisorCode == 0)
        mFrequency = 524288 * 2;
    else
        mFrequency = 524288 / mDivisorCode;
    mPeriod = mFrequency / (2 << mShiftClock);
    mPeriod = GENERAL_FREQUENCY / mPeriod;
    NR43 = a_value;
}

void cSoundChannel4::writeNR44(int a_value)
{
    mConsecutive = (a_value & 0x40) == 0;
    if ((a_value & 0x80) != 0) // Trigger
    {
        mOnOff = true;
        mSoundLength = (mSoundLength == 0) ? 64 * (CYCLES_PER_SECOND / 256) : mSoundLength;
        mInitialVolumen = NR42 >> 4;
        if (mInitialVolumen == 0)
            mOnOff = false;
        mPatternIndex = 0;
        mCounter = 0;
    }
    NR44 = a_value | 0xBF;
}

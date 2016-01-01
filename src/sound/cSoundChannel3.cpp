//
// Created by dospro on 14/12/15.
//

#include "cSoundChannel3.h"

cSoundChannel3::cSoundChannel3(int a_generalFrecuency) :
        cSoundChannel(a_generalFrecuency),
        mCounter{0},
        mPatternIndex{0}
{
    for (int i = 0; i < 32; i += 4)
    {
        mWaveRam[i] = 0;
        mWaveRam[i + 1] = 0;
        mWaveRam[i + 2] = 0xFF;
        mWaveRam[i + 3] = 0xFF;
    }
}

cSoundChannel3::~cSoundChannel3()
{

}

int cSoundChannel3::readRegister(int a_address)
{
    switch (a_address)
    {
        case 0xFF1A:
            return NR30 | 0x7F;
        case 0xFF1B:
            return NR31 | 0xFF;
        case 0xFF1C:
            return NR32 | 0x9F;
        case 0xFF1D:
            return NR33 | 0xFF;
        case 0xFF1E:
            return NR34 | 0xBF;
        default:
            if (a_address >= 0xFF30 && a_address < 0xFF40)
                return mWaveLastWrittenValue;
            return 0xFF;
    }
}

void cSoundChannel3::writeRegister(int a_address, int a_value)
{
    switch (a_address)
    {
        case 0xFF1A:
            writeNR30(a_value);
            break;
        case 0xFF1B:
            writeNR31(a_value);
            break;
        case 0xFF1C:
            writeNR32(a_value);
            break;
        case 0xFF1D:
            writeNR33(a_value);
            break;
        case 0xFF1E:
            writeNR34(a_value);
            break;
        default:
            if (a_address >= 0xFF30 && a_address < 0xFF40)
            {
                writeWaveRam(a_address, a_value);
            }
            break;
    }
}

void cSoundChannel3::writeNR30(int a_value)
{
    if ((a_value & 0x80) == 0)
    {
        mDACBit = false;
        mOnOff = false;
    }
    else
        mDACBit = true;
    NR30 = a_value;
}

void cSoundChannel3::writeNR31(int a_value)
{
    mSoundLength = (256 - a_value) * (CYCLES_PER_SECOND / 256);
    NR31 = a_value;
}

void cSoundChannel3::writeNR32(int a_value)
{
    mOutputLevel = (a_value >> 5) & 3;
    switch (mOutputLevel)
    {
        case 0:
            mOutputLevel = 8;
            break;
        case 1:
            mOutputLevel = 0;
            break;
        case 2:
            mOutputLevel = 1;
            break;
        case 3:
            mOutputLevel = 2;
        default:
            break;
    }
    NR32 = a_value;
}

void cSoundChannel3::writeNR33(int a_value)
{
    mFrequency = (mFrequency & 0x700) | a_value;
    setFrequency();
    NR33 = a_value;
}

void cSoundChannel3::writeNR34(int a_value)
{
    mFrequency = (mFrequency & 0xFF) | ((a_value & 7) << 8);
    setFrequency();
    mConsecutive = (a_value & 0x40) == 0;
    if ((a_value & 0x80) != 0) // Trigger
    {
        mSoundLength = mSoundLength == 0 ? CYCLES_PER_SECOND : mSoundLength;
        mPatternIndex = 0;
        mCounter = 0.0f;
        mOnOff = mDACBit;
    }
    NR34 = a_value;
}

void cSoundChannel3::writeWaveRam(int a_address, int a_value)
{
    int index = a_address - 0xFF30;
    mWaveRam[index * 2] = (a_value >> 4) << 4;
    mWaveRam[index * 2 + 1] = (a_value & 0xF) << 4;
    mWaveLastWrittenValue = a_value;
}

int cSoundChannel3::getOnOffBit()
{
    if (mOnOff)
        return 4;
    return 0;
}

int cSoundChannel3::getSample()
{
    if (mCounter <= mSamplePerSecond)
    {
        ++mCounter;
    }
    else
    {
        ++mPatternIndex;
        if (mPatternIndex >= mWaveRam.size())
            mPatternIndex = 0;
        mCounter -= mSamplePerSecond - 1;

    }
    if (!outputTerminal1 && !outputTerminal2)
        return 0;
    return mWaveRam[mPatternIndex] >> mOutputLevel;
}

void cSoundChannel3::update(int a_cycles)
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
}

void cSoundChannel3::setFrequency()
{
    double finalFrequency = {65536.0f / (2048.0f - mFrequency)};
    mSamplePerSecond = GENERAL_FREQUENCY / finalFrequency / mWaveRam.size();
}
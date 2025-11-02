#include "sound_channel_2.h"

cSoundChannel2::cSoundChannel2(int a_generalFrequency) : cSoundChannel(a_generalFrequency) {
}

int cSoundChannel2::readRegister(const int a_address) const {
    switch (a_address) {
        case 0xFF16:
            return NR21 | 0x3F;
        case 0xFF17:
            return NR22;
        case 0xFF18:
            return NR23 | 0xFF;
        case 0xFF19:
            return NR24 | 0xBF;
        default:
            return 0xFF;
    }
}

void cSoundChannel2::writeRegister(const int a_address, const int a_value) {
    switch (a_address) {
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

int cSoundChannel2::getOnOffBit() const {
    if (mOnOff)
        return 2;
    return 0;
}

void cSoundChannel2::writeNR21(const int a_value) {
    mWavePatternType = a_value >> 6;
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
    NR21 = a_value;
}

void cSoundChannel2::writeNR22(const int a_value) {
    mInitialVolumen = a_value >> 4;
    mIncreaseVolumen = ((a_value & 8) != 0);
    setEnvelopTimer(a_value);
    NR22 = a_value;
}

void cSoundChannel2::writeNR23(const int a_value) {
    mFrequency = (mFrequency & 0x700) | a_value;
    setFrequency();
    NR23 = a_value;
}

void cSoundChannel2::writeNR24(const int a_value) {
    mFrequency = (mFrequency & 0xFF) | ((a_value & 7) << 8);
    setFrequency();
    mConsecutive = ((a_value & 0x40) == 0);
    if ((a_value & 0x80) != 0) // Trigger
    {
        mOnOff = true;
        mSoundLength = (mSoundLength == 0) ? 64 * (CYCLES_PER_SECOND / 256) : mSoundLength;
        mCounter = 0;
        mPatternIndex = 0;
        setEnvelopTimer(NR22);
        mInitialVolumen = NR22 >> 4;
        if ((NR22 & 0xF8) == 0)
            mOnOff = false;
    }
    NR24 = a_value;
}

int cSoundChannel2::getSample() {
    if (mCounter <= mSamplesPerSecond) {
        ++mCounter;
    } else {
        ++mPatternIndex;
        if (mPatternIndex >= mWavePatternDuty[mWavePatternType].size())
            mPatternIndex = 0;
        mCounter -= mSamplesPerSecond - 1;
    }
    if (!outputTerminal1 && !outputTerminal2)
        return 0;
    return (mInitialVolumen * mWavePatternDuty[mWavePatternType][mPatternIndex]) / 15;
}

void cSoundChannel2::update(const int a_cycles) {
    if (mOnOff && !mConsecutive) {
        mSoundLength -= a_cycles;
        if (mSoundLength <= 0) {
            mOnOff = false;
            mSoundLength = 0;
        }
    }
    if (mOnOff && mEnvelopSweep != 0) {
        mEnvelopSweep -= a_cycles;
        if (mEnvelopSweep <= 0) {
            if (mIncreaseVolumen) {
                mInitialVolumen++;
                if (mInitialVolumen > 0xF)
                    mInitialVolumen = 0xF;
            } else {
                mInitialVolumen--;
                if (mInitialVolumen < 0)
                    mInitialVolumen = 0;
            }
            mEnvelopSweep = (NR22 & 7) * (CYCLES_PER_SECOND / 64);
        }
    }
}

void cSoundChannel2::setFrequency() {
    const double finalFrequency = 131072.0f / (2048.0f - static_cast<double>(mFrequency));
    mSamplesPerSecond = GENERAL_FREQUENCY / finalFrequency / static_cast<double>(mWavePatternDuty[mWavePatternType].size());
}

void cSoundChannel2::setEnvelopTimer(const int a_value) {
    mEnvelopSweep = (a_value & 7) * (CYCLES_PER_SECOND / 64);
}

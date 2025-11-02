#include "sound_channel_1.h"

cSoundChannel1::cSoundChannel1(int a_generalFrequency) : cSoundChannel(a_generalFrequency) {
}


int cSoundChannel1::readRegister(const int a_address) const {
    switch (a_address) {
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

void cSoundChannel1::writeRegister(const int a_address, const int a_value) {
    switch (a_address) {
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

int cSoundChannel1::getOnOffBit() const {
    if (mOnOff)
        return 1;
    return 0;
}

int cSoundChannel1::getSample() {
    mCounter++;
    if (mCounter > mSamplesPerSecond) {
        ++mPatternIndex;
        if (mPatternIndex >= mWavePatternDuty[mWavePatternType].size())
            mPatternIndex = 0;
        mCounter -= mSamplesPerSecond;
    }
    if (!outputTerminal1 && !outputTerminal2)
        return 0;
    return (mInitialVolumen * mWavePatternDuty[mWavePatternType][mPatternIndex]) / 15;
}

void cSoundChannel1::update(const int a_cycles) {
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
            if (mVolumenIncrease) {
                mInitialVolumen++;
                if (mInitialVolumen > 0xF)
                    mInitialVolumen = 0xF;
            } else {
                mInitialVolumen--;
                if (mInitialVolumen < 0)
                    mInitialVolumen = 0;
            }
            setEnvelopTimer(NR12);
        }
    }

    if (mOnOff && mSweepEnabled) {
        mSweepTime -= a_cycles;
        if (mSweepTime <= 0) {
            if (mSweepIncrement) {
                mShadowFrequency += mShadowFrequency >> (mSweepShift + 1);
                if (mShadowFrequency > 2047) {
                    mShadowFrequency = 2047;
                    mOnOff = false;
                    mSweepEnabled = false;
                } else {
                    if (mSweepShift != 0) {
                        NR13 = mShadowFrequency & 0xFF;
                        NR14 = (NR14 & 0xF8) | ((mShadowFrequency >> 8) & 7);
                        mFrequency = mShadowFrequency;
                        setFrequency();
                        calculateNewFrequency();
                    }
                    setSweepTimer(NR10);
                }
            } else {
                mShadowFrequency -= mShadowFrequency >> (mSweepShift + 1);
                if (mShadowFrequency < 0)
                    mShadowFrequency = 0;
                else {
                    if (mSweepShift != 0) {
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

void cSoundChannel1::writeNR10(const int a_value) {
    setSweepTimer(a_value);
    mSweepIncrement = ((a_value >> 3) & 1) == 0;
    mSweepShift = a_value & 7;
    mSweepEnabled = false;
    NR10 = a_value;
}

void cSoundChannel1::writeNR11(const int a_value) {
    mWavePatternType = a_value >> 6;
    setLengthTimer(a_value);
    NR11 = a_value;
}

void cSoundChannel1::writeNR12(const int a_value) {
    mInitialVolumen = a_value >> 4;
    mVolumenIncrease = ((a_value >> 3) & 1) != 0;
    setEnvelopTimer(a_value);
    NR12 = a_value;
}

void cSoundChannel1::writeNR13(const int a_value) {
    mFrequency = (mFrequency & 0x700) | a_value;
    setFrequency();
    NR13 = a_value;
}

void cSoundChannel1::writeNR14(const int a_value) {
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

void cSoundChannel1::sweepTrigger() {
    mShadowFrequency = mFrequency;
    setSweepTimer(NR10);
    mSweepEnabled = true;
    if (mSweepTime == 0 && mSweepShift == 0)
        mSweepEnabled = false;

    if (mSweepShift != 0) {
        calculateNewFrequency();
    }
}

void cSoundChannel1::calculateNewFrequency() {
    const int freqIncrement = mShadowFrequency >> (mSweepShift + 1);
    mShadowFrequency += (mSweepIncrement ? freqIncrement : -freqIncrement);
    if (mShadowFrequency > 2047) {
        mShadowFrequency = 2047;
        mOnOff = false;
    } else if (mShadowFrequency <= 0)
        mShadowFrequency = 0;
}

void cSoundChannel1::setSweepTimer(const int a_value) {
    mSweepTime = (a_value & 0x70) >> 4;
    //if (mSweepTime == 0)
    //    mSweepTime = 8;
    mSweepTime *= (CYCLES_PER_SECOND / 128);
}

void cSoundChannel1::setLengthTimer(const int a_value) {
    mSoundLength = (64 - (a_value & 63)) * (CYCLES_PER_SECOND / 256);
}

void cSoundChannel1::setEnvelopTimer(const int a_value) {
    mEnvelopSweep = a_value & 7;
    //if (mEnvelopSweep == 0)
    //    mEnvelopSweep = 8;
    mEnvelopSweep *= (CYCLES_PER_SECOND / 64);
}

void cSoundChannel1::setFrequency() {
    const double finalFrequency = 131072.0f / (2048.0f - static_cast<double>(mFrequency));
    mSamplesPerSecond = GENERAL_FREQUENCY / finalFrequency / static_cast<double>(mWavePatternDuty[mWavePatternType].size());
}

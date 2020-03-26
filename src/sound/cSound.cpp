#include"cSound.h"

cSound::cSound(int a_generalFrequency) :
        GENERAL_FREQUENCY{a_generalFrequency},
        NUMBER_OF_CHANNELS{2}
{
    mChannel1 = std::unique_ptr<cSoundChannel1>(new cSoundChannel1(GENERAL_FREQUENCY));
    mChannel2 = std::unique_ptr<cSoundChannel2>(new cSoundChannel2(GENERAL_FREQUENCY));
    mChannel3 = std::unique_ptr<cSoundChannel3>(new cSoundChannel3(GENERAL_FREQUENCY));
    mChannel4 = std::unique_ptr<cSoundChannel4>(new cSoundChannel4(GENERAL_FREQUENCY));

    soundActive = true;
}

cSound::~cSound()
{
}

int cSound::readFromSound(int a_address)
{
    if (a_address < 0xFF16)
        return mChannel1->readRegister(a_address);
    else if (a_address < 0xFF1A)
        return mChannel2->readRegister(a_address);
    else if (a_address < 0xFF20)
        return mChannel3->readRegister(a_address);
    else if (a_address < 0xFF24)
        return mChannel4->readRegister(a_address);
    else if (a_address < 0xFF30)
    {
        switch (a_address)
        {
            case 0xFF24: //NR 50
                return NR50;
            case 0xFF25:
                return NR51;
            case 0xFF26: //NR 52
                return static_cast<u8>(mChannel1->getOnOffBit()
                                       | mChannel2->getOnOffBit()
                                       | mChannel3->getOnOffBit()
                                       | mChannel4->getOnOffBit()
                                       | (soundActive << 7)
                                       | 0x70);
            default:
                return 0xFF;
        }
    }
    else
        return mChannel3->readRegister(a_address);
}

void cSound::writeToSound(u16 address, u8 value)
{
    switch (address)
    {
        case 0xFF10: //NR 10
        case 0xFF11: //NR 11
        case 0xFF12: //NR 12
        case 0xFF13: //NR 13
        case 0xFF14: //NR 14
            mChannel1->writeRegister(address, value);
        case 0xFF16: //NR 21
        case 0xFF17: //NR 22
        case 0xFF18: //NR 23
        case 0xFF19: //NR 24
            mChannel2->writeRegister(address, value);
        case 0xFF1A: //NR 30
        case 0xFF1B: //NR 31
        case 0xFF1C: //NR 32
        case 0xFF1D: //NR 33
        case 0xFF1E: //NR 34
            mChannel3->writeRegister(address, value);
        case 0xFF20: //NR 41
        case 0xFF21: //NR 42
        case 0xFF22: //NR 43
        case 0xFF23: //NR 44
            mChannel4->writeRegister(address, value);
        case 0xFF24: //NR 50
            NR50 = value;
            break;
        case 0xFF25:
            mChannel1->setOutputTerminal((value & 1) != 0, (value & 0x10) != 0);
            mChannel2->setOutputTerminal((value & 2) != 0, (value & 0x20) != 0);
            mChannel3->setOutputTerminal((value & 4) != 0, (value & 0x40) != 0);
            mChannel4->setOutputTerminal((value & 8) != 0, (value & 0x80) != 0);
            NR51 = value;
            break;
        case 0xFF26: //NR 52
            soundActive = ((value & 0x80) != 0);
            break;
        default:
            if (address >= 0xFF30 && address < 0xFF40)
            {
                mChannel3->writeRegister(address, value);
            }
            break;
    }
}

void cSound::fillBuffer(u8 *a_internalBuffer, int a_bufferSize)
{
    int sample1, sample2, sample3, sample4;
    int leftFinalSample;
    int rightFinalSample;

    for (int i = 0; i < a_bufferSize / NUMBER_OF_CHANNELS; ++i)
    {
        sample1 = 0;
        sample2 = 0;
        sample3 = 0;
        sample4 = 0;
        if (soundActive)
        {
            if (mChannel1->isOn())
                sample1 = mChannel1->getSample();
            if (mChannel2->isOn())
                sample2 = mChannel2->getSample();
            if (mChannel3->isOn())
                sample3 = mChannel3->getSample();
            if (mChannel4->isOn())
                sample4 = mChannel4->getSample();
        }
        sample1 -= 128;
        sample2 -= 128;
        sample3 -= 128;
        sample4 -= 128;

        leftFinalSample = 0;
        if (mChannel1->isLeftSound())
            leftFinalSample += sample1;
        if (mChannel2->isLeftSound())
            leftFinalSample += sample2;
        if (mChannel3->isLeftSound())
            leftFinalSample += sample3;
        if (mChannel4->isLeftSound())
            leftFinalSample += sample4;
        leftFinalSample >>= 4;

        rightFinalSample = 0;
        if (mChannel1->isRightSound())
            rightFinalSample += sample1;
        if (mChannel2->isRightSound())
            rightFinalSample += sample2;
        if (mChannel3->isRightSound())
            rightFinalSample += sample3;
        if (mChannel4->isRightSound())
            rightFinalSample += sample4;
        rightFinalSample >>= 4;


        a_internalBuffer[i * 2] = static_cast<u8>((leftFinalSample + 128) & 0xFF);
        a_internalBuffer[i * 2 + 1] = static_cast<u8>((rightFinalSample + 128) & 0xFF);
    }
}

void cSound::updateCycles(s32 cycles)
{
    mChannel1->update(cycles);
    mChannel2->update(cycles);
    mChannel3->update(cycles);
    mChannel4->update(cycles);
}


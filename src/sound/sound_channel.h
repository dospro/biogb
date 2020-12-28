//
// Created by dospro on 10/12/15.
//

#ifndef BIOGB_SOUND_CHANNEL_H
#define BIOGB_SOUND_CHANNEL_H

#include <array>

class cSoundChannel
{
public:
    cSoundChannel(int a_generalFrecuency);
    virtual ~cSoundChannel();
    virtual int readRegister(int a_address) = 0;
    virtual void writeRegister(int a_address, int a_value) = 0;
    virtual int getSample() = 0;
    virtual void update(int a_cycles) = 0;
    virtual int getOnOffBit() = 0;
    virtual void setOutputTerminal(bool a_left, bool a_right);
    virtual bool isLeftSound();
    virtual bool isRightSound();
    virtual bool isOn();


protected:
    const int CYCLES_PER_SECOND = 4194304;
    const int GENERAL_FREQUENCY;
    std::array<std::array<int, 8>, 4> mWavePatternDuty;
    bool mOnOff;
    bool outputTerminal1, outputTerminal2;

};

#endif  // BIOGB_SOUND_CHANNEL_H

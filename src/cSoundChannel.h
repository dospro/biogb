//
// Created by dospro on 10/12/15.
//

#ifndef BIOGB_CSOUNDCHANNEL_H
#define BIOGB_CSOUNDCHANNEL_H


#include <array>

class cSoundChannel
{
public:
    cSoundChannel(int a_generalFrecuency);
    virtual ~cSoundChannel();
    virtual int getSample() = 0;
    virtual void update(int a_cycles) = 0;

protected:
    const int CYCLES_PER_SECOND = 4194304;
    const int GENERAL_FREQUENCY;
    std::array<std::array<int, 8>, 4> mWavePatternDuty;
    bool mOnOff;
    bool outputTerminal1, outputTerminal2;

};

#endif //BIOGB_CSOUNDCHANNEL_H

//
// Created by dospro on 10/12/15.
//


#include "cSoundChannel.h"


cSoundChannel::cSoundChannel(int a_generalFrecuency) :
        GENERAL_FREQUENCY{a_generalFrecuency},
        mOnOff{false}
{
    mWavePatternDuty[0][0] = 0;
    mWavePatternDuty[0][1] = 0xFF;
    mWavePatternDuty[0][2] = 0xFF;
    mWavePatternDuty[0][3] = 0xFF;
    mWavePatternDuty[0][4] = 0xFF;
    mWavePatternDuty[0][5] = 0xFF;
    mWavePatternDuty[0][6] = 0xFF;
    mWavePatternDuty[0][7] = 0xFF;


    mWavePatternDuty[1][0] = 0;
    mWavePatternDuty[1][1] = 0;
    mWavePatternDuty[1][2] = 0xFF;
    mWavePatternDuty[1][3] = 0xFF;
    mWavePatternDuty[1][4] = 0xFF;
    mWavePatternDuty[1][5] = 0xFF;
    mWavePatternDuty[1][6] = 0xFF;
    mWavePatternDuty[1][7] = 0xFF;

    mWavePatternDuty[2][0] = 0;
    mWavePatternDuty[2][1] = 0;
    mWavePatternDuty[2][2] = 0;
    mWavePatternDuty[2][3] = 0;
    mWavePatternDuty[2][4] = 0xFF;
    mWavePatternDuty[2][5] = 0xFF;
    mWavePatternDuty[2][6] = 0xFF;
    mWavePatternDuty[2][7] = 0xFF;

    mWavePatternDuty[3][0] = 0;
    mWavePatternDuty[3][1] = 0;
    mWavePatternDuty[3][2] = 0;
    mWavePatternDuty[3][3] = 0;
    mWavePatternDuty[3][4] = 0;
    mWavePatternDuty[3][5] = 0;
    mWavePatternDuty[3][6] = 0xFF;
    mWavePatternDuty[3][7] = 0xFF;
}

cSoundChannel::~cSoundChannel()
{

}

bool cSoundChannel::isOn()
{
    return mOnOff;
}

void cSoundChannel::setOutputTerminal(bool a_left, bool a_right)
{
    outputTerminal1 = a_left;
    outputTerminal2 = a_right;
}

bool cSoundChannel::isLeftSound()
{
    return outputTerminal1;
}

bool cSoundChannel::isRightSound()
{
    return outputTerminal2;
}

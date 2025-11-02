#include "sound_channel.h"

cSoundChannel::cSoundChannel(const int a_generalFrequency) : GENERAL_FREQUENCY{a_generalFrequency} {
}

bool cSoundChannel::isOn() const noexcept {
    return mOnOff;
}

void cSoundChannel::setOutputTerminal(const bool a_left, const bool a_right) {
    outputTerminal1 = a_left;
    outputTerminal2 = a_right;
}

bool cSoundChannel::isLeftSound() const noexcept {
    return outputTerminal1;
}

bool cSoundChannel::isRightSound() const noexcept {
    return outputTerminal2;
}

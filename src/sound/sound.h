#ifndef BIOGB_SOUND
#define BIOGB_SOUND

#include "../macros.h"
#include "sound_channel_1.h"
#include "sound_channel_2.h"
#include "sound_channel_3.h"
#include "sound_channel_4.h"

class cSound
{
public:
    explicit cSound(int a_generalFrequency);
    virtual ~cSound() = default;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    [[nodiscard]] u8 readFromSound(u16 a_address) const;
    void writeToSound(u16 address, u8 value);
    void fillBuffer(u8 *a_internalBuffer, int a_bufferSize);
    void updateCycles(s32 cycles);
protected:
    cSoundChannel1 mChannel1;
    cSoundChannel2 mChannel2;
    cSoundChannel3 mChannel3;
    cSoundChannel4 mChannel4;
    const int GENERAL_FREQUENCY{};
    const int NUMBER_OF_CHANNELS{};
    bool soundActive{};
    u8 NR50{};
    u8 NR51{};

};

#endif

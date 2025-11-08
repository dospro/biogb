#include "input.h"

void cInput::update_input(const GBKey gb_key) {
    switch (gb_key) {
        case GBKey::Up: mDirections |= 4;
            break;
        case GBKey::Down: mDirections |= 8;
            break;
        case GBKey::Left: mDirections |= 2;
            break;
        case GBKey::Right: mDirections |= 1;
            break;
        case GBKey::A: mButtons |= 1;
            break;
        case GBKey::B: mButtons |= 2;
            break;
        case GBKey::Start: mButtons |= 8;
            break;
        case GBKey::Select: mButtons |= 4;
    }
}

void cInput::reset_input() {
    mDirections &= 0xF0;
    mButtons &= 0xF0;
}

int cInput::readRegister() const {
    return mP1Value;
}

void cInput::writeRegister(const int a_value) {
    if ((a_value & 16) == 0) //Directions
        mP1Value = ((mDirections ^ 255) & 0xF) | 0xE0;
    else if ((a_value & 32) == 0) //Buttons
        mP1Value = ((mButtons ^ 255) & 0xF) | 0xD0;
    else
        mP1Value = 0xFF;
}

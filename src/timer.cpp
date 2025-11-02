#include "timer.h"

int cTimer::readRegister(const int a_address) const {
    switch (a_address) {
        case 0xFF04:
            return mDIV >> 8;
        case 0xFF05:
            return mTIMA;
        case 0xFF06:
            return mTMA;
        case 0xFF07:
            return mTAC;
        default:
            return 0xFF;
    }
}

void cTimer::writeRegister(const int a_address, const int a_value) {
    switch (a_address) {
        case 0xFF04:
            mDIV = 0;
            break;
        case 0xFF05:
            mTIMA = a_value;
            break;
        case 0xFF06:
            mTMA = a_value;
            break;
        case 0xFF07:
            mTAC = a_value;
        default:
            break;
    }
}

void cTimer::update(const int a_cycles) {
    int newDiv = (mDIV + a_cycles) & 0xFFFF;
    int carryBit{0};
    int freq{mTAC & 3};
    int enable{(mTAC >> 2) & 1};

    switch (freq) {
        case 0: carryBit = 0x200; break;
        case 3: carryBit = 0x80; break;
        case 2: carryBit = 0x20; break;
        case 1: carryBit = 0x8; break;
        default: break;
    }
    if (enable != 0) {
        // Falling-edge detector (went from 1 to 0)
        if ((mDIV & carryBit) != 0 && (newDiv & carryBit) == 0) mTIMA++;
    }

    if (mTIMA > 0xFF) {
        mTIMA = mTMA;
        InterruptBit = true;
    }
    mDIV = newDiv;
}

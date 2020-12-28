#include "timer.h"

cTimer::cTimer() : mDIV{0}, mTAC{0}, mTIMA{0}, mTMA{0}, InterruptBit{false} {
}

cTimer::~cTimer() = default;

int cTimer::readRegister(int a_address) {
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

void cTimer::writeRegister(int a_address, int a_value) {
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

void cTimer::update(int a_cycles) {
    int newDiv = (mDIV + a_cycles) & 0xFFFF;
    int carryBits = mDIV ^newDiv;
    int freq{mTAC & 3};
    int enable{(mTAC >> 2) & 1};

    switch (freq) {
        case 0:
            carryBits >>= 9;
            break;
        case 3:
            carryBits >>= 7;
            break;
        case 2:
            carryBits >>= 5;
            break;
        case 1:
            carryBits >>= 3;
            break;
        default:
            break;
    }
    if (enable != 0) {
        if (carryBits != 0)
            mTIMA++;
    }

    if (mTIMA > 0xFF)//This means we will have an overflow
    {
        mTIMA = mTMA;
        InterruptBit = true;
    }
    mDIV = newDiv;
}

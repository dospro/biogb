#include "cInterrupts.h"

cInterrupts::cInterrupts() : mIERegister{0}, mIFRegister{0} {}

cInterrupts::~cInterrupts() {}

int cInterrupts::readRegister(int a_address) {
    if (a_address == 0xFF0F)
        return mIFRegister;
    else
        return mIERegister;
}

void cInterrupts::writeRegister(int a_address, int a_value) {
    if (a_address == 0xFF0F)
        mIFRegister = a_value;
    else
        mIERegister = a_value;
}

int cInterrupts::getReadyInterrupts() {
    return mIFRegister & mIERegister;
}

void cInterrupts::setInterrupt(int a_interruptBit) {
    mIFRegister |= a_interruptBit;
}

void cInterrupts::resetInterrupt(int a_interruptBit) {
    mIFRegister &= (a_interruptBit ^ 0xFF);
}

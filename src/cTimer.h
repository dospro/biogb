#ifndef BIOGB_CTIMER_H
#define BIOGB_CTIMER_H


#include "MemoryMap.h"

class cTimer {
public:
    cTimer();
    ~cTimer();
    int readRegister(int a_address);
    void writeRegister(int a_address, int a_value);
    void update(int a_cycles);

    bool InterruptBit;
private:
    int mDIV;
    int mTAC;
    int mTIMA;
    int mTMA;
    int mCounter;
    int mOldBit;
};


#endif //BIOGB_CTIMER_H

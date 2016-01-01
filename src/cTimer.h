//
// Created by dospro on 28/12/15.
//

#ifndef BIOGB_CTIMER_H
#define BIOGB_CTIMER_H


#include "cInterrupts.h"

class cTimer
{
public:
    cTimer(cInterrupts *a_interruptPointer);
    ~cTimer();
    int readRegister(int a_address);
    void writeRegister(int a_address, int a_value);
    void update(int a_cycles);
private:
    cInterrupts *mInterrupts;
    int mDIV;
    int mTAC;
    int mTIMA;
    int mTMA;
    int mCounter;
    int mOldBit;
};


#endif //BIOGB_CTIMER_H

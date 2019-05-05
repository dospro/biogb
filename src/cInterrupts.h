#ifndef BIOGB_CINTERRUPTS_H
#define BIOGB_CINTERRUPTS_H


class cInterrupts {
public:
    enum {
        VBLANK = 1,
        LCDC = 2,
        TIMER = 4,
        SERIAL = 8,
        JOYPAD = 16
    };
    cInterrupts();
    ~cInterrupts();
    int readRegister(int a_address);
    void writeRegister(int a_address, int a_value);
    int getReadyInterrupts();
    void setInterrupt(int a_interruptBit);
    void resetInterrupt(int a_interruptBit);

private:
    int mIFRegister;
    int mIERegister;
};


#endif //BIOGB_CINTERRUPTS_H

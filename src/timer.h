#ifndef BIOGB_TIMER_H
#define BIOGB_TIMER_H

class cTimer {
public:
    cTimer() = default;
    ~cTimer() = default;
    int readRegister(int a_address) const;
    void writeRegister(int a_address, int a_value);
    void update(int a_cycles);

    bool InterruptBit{};
private:
    int mDIV{};
    int mTAC{};
    int mTIMA{};
    int mTMA{};
    int mCounter{};
    int mOldBit{};
};


#endif  // BIOGB_TIMER_H

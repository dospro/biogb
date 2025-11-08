#ifndef BIOGB_INPUT
#define BIOGB_INPUT


enum class GBKey : int {
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
    A = 4,
    B = 5,
    Start = 6,
    Select = 7,
};

class cInput {
public:
    cInput() = default;

    ~cInput() = default;

    void reset_input();
    void update_input(GBKey key);

    int readRegister() const;

    void writeRegister(int a_value);

private:
    int mButtons{};
    int mDirections{};
    int mP1Value{};
};

#endif

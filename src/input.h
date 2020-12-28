#ifndef BIOGB_INPUT
#define BIOGB_INPUT


#include<SDL.h>


#define GBK_ESCAPE SDL_SCANCODE_ESCAPE
#define GBK_SPACE SDL_SCANCODE_SPACE
#define GBK_KP_MINUS SDL_SCANCODE_KP_MINUS
#define GBK_KP_PLUS SDL_SCANCODE_KP_PLUS
#define GBK_s SDL_SCANCODE_S
#define GBK_l SDL_SCANCODE_L
#define GBK_x SDL_SCANCODE_X
#define GBK_z SDL_SCANCODE_Z
#define GBK_UP SDL_SCANCODE_UP
#define GBK_DOWN SDL_SCANCODE_DOWN
#define GBK_LEFT SDL_SCANCODE_LEFT
#define GBK_RIGHT SDL_SCANCODE_RIGHT
#define GBK_RETURN SDL_SCANCODE_RETURN
#define GBK_RSHIFT SDL_SCANCODE_RSHIFT


#define GB_UP        0
#define GB_DOWN        1
#define GB_LEFT        2
#define GB_RIGHT    3
#define GB_A        4
#define GB_B        5
#define GB_START    6
#define GB_SELECT    7

class cInput
{
public:

    cInput();
    ~cInput();
    void setKey(int gbKey, int pKey);
    void update();
    bool isKeyPressed(int k);
    bool isGbKeyPressed(int k);
    bool isButtonPressed(int b);
    int readRegister();
    void writeRegister(int a_value);


private:
    int mButtons;
    int mDirections;
    int mP1Value;
    SDL_Event event;
    const unsigned char *key; //keyboard

    int gbky[9]; //Gameboy keys
    unsigned int button; //joypad/joystick

};

#endif

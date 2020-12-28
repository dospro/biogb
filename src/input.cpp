#include "input.h"

#include <stdexcept>

cInput::cInput()
{
    if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
    {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
            throw std::runtime_error("SDL Error can't init input system.");
    }

    gbky[GB_UP] = GBK_UP;
    gbky[GB_DOWN] = GBK_DOWN;
    gbky[GB_LEFT] = GBK_LEFT;
    gbky[GB_RIGHT] = GBK_RIGHT;
    gbky[GB_A] = GBK_z;
    gbky[GB_B] = GBK_x;
    gbky[GB_START] = GBK_RETURN;
    gbky[GB_SELECT] = GBK_RSHIFT;
}

cInput::~cInput()
{

}

bool cInput::isButtonPressed(int b)
{
    return false;
}

bool cInput::isKeyPressed(int k)
{
    return key[k] != 0;
}

bool cInput::isGbKeyPressed(int k)
{
    return key[gbky[k]] != 0;
}

void cInput::update()
{
    SDL_PumpEvents();
    key = SDL_GetKeyboardState(nullptr);

    if (isGbKeyPressed(GB_UP)) mDirections |= 4;
    else mDirections &= 251;
    if (isGbKeyPressed(GB_DOWN)) mDirections |= 8;
    else mDirections &= 247;
    if (isGbKeyPressed(GB_LEFT)) mDirections |= 2;
    else mDirections &= 253;
    if (isGbKeyPressed(GB_RIGHT)) mDirections |= 1;
    else mDirections &= 254;

    if (isGbKeyPressed(GB_A)) mButtons |= 1;
    else mButtons &= 254;
    if (isGbKeyPressed(GB_B)) mButtons |= 2;
    else mButtons &= 253;
    if (isGbKeyPressed(GB_START)) mButtons |= 8;
    else mButtons &= 247;
    if (isGbKeyPressed(GB_SELECT)) mButtons |= 4;
    else mButtons &= 251;
}

int cInput::readRegister()
{
    return mP1Value;
}

void cInput::writeRegister(int a_value)
{
    if ((a_value & 16) == 0)//Directions
        mP1Value = ((mDirections ^ 255) & 0xF) | 0xE0;
    else if ((a_value & 32) == 0)//Buttons
        mP1Value = ((mButtons ^ 255) & 0xF) | 0xD0;
    else
        mP1Value = 0xFF;
}

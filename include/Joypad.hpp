#pragma once
#include "types.hpp"

struct JoypadState{
    uint8_t Joypad{0xFF};
    uint8_t SB;
    uint8_t SC;
};

class Joypad{
    JoypadState self;
    InterruptState* IS;

    uint8_t prevButtonState = 0xFF;
public:
    uint8_t directions{0xFF};
    uint8_t buttons{0xFF};
    
    void update();
    JoypadState* get();
    void setInterrupt(InterruptState* master);
};
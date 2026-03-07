#pragma once
#include "types.hpp"

class Joypad{
    InterruptState* IS;
    uint8_t Joypad{0xFF};
public:
    uint8_t directions{0xF};
    uint8_t buttons{0xF};
    
    void update();

    bool write(uint16_t addr, uint8_t data);
    bool read(uint16_t addr, uint8_t& data);
    
    void setInterrupt(InterruptState* master);
};

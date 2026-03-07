#pragma once
#include "types.hpp"

class Joypad{
    uint8_t Joypad{0xFF};
    InterruptState* IS;
public:
    uint8_t directions{0xFF};
    uint8_t buttons{0xFF};
    
    void update();

    bool write(uint16_t addr, uint8_t data);
    bool read(uint16_t addr, uint8_t& data);
    
    void setInterrupt(InterruptState* master);
};
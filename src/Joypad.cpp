#include "../include/Joypad.hpp"

void Joypad::update(){
    uint8_t result = Joypad & 0xF0;
    if (!(Joypad & 0x20)) {
        result |= buttons;
    }
    if (!(Joypad & 0x10)) {
        result |= directions;
    }
    
    Joypad = result;
    IS->IF |= INPUT;
}

bool Joypad::write(uint16_t addr, uint8_t data){
    if (addr == 0xFF00){
        Joypad = 0xC0 | (data&0xF0) | (Joypad&0xF);
        update();
        return true;
    }
    return false;
}
bool Joypad::read(uint16_t addr, uint8_t& data){
    if (addr == 0xFF00){
        data = Joypad;
        return true;
    }
    return false;
}
void Joypad::setInterrupt(InterruptState* master){
    IS = master;
}

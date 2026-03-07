#include "../include/Joypad.hpp"

void Joypad::update(){
    uint8_t result = 0xC0 | (Joypad & 0xF0);
    if (!(Joypad & 0x20)) {
        result |= (buttons & 0xF);
    }
    else if (!(Joypad & 0x10)) {
        result |= (directions & 0xF);
    }
    
    Joypad = result;
    IS->IF |= INPUT;
}

bool Joypad::write(uint16_t addr, uint8_t data){
    if (addr == 0xFF00){
        Joypad = (data&0xF0) | (Joypad&0xF);
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
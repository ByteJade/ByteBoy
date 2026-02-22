#include "../include/Joypad.hpp"

void Joypad::update(){
    uint8_t currentButtonState = 0xFF;
    
    currentButtonState = (directions << 4) | buttons;
    
    uint8_t result = 0xC0 | (self.Joypad & 0xF0);
    if (!(self.Joypad & 0x20)) {
        result |= (buttons & 0xF);
    }
    else if (!(self.Joypad & 0x10)) {
        result |= (directions & 0xF);
    }
    
    self.Joypad = result;
    if (prevButtonState ^ currentButtonState) {
        IS->IF |= INPUT;
    }
    prevButtonState = currentButtonState;
}

bool Joypad::write(uint16_t addr, uint8_t data){
    if (addr == 0xFF00){
        self.Joypad = (data&0xF0) | (self.Joypad&0xF);
        update();
        return true;
    }
    return false;
}
bool Joypad::read(uint16_t addr, uint8_t& data){
    if (addr == 0xFF00){
        data = self.Joypad;
        return true;
    }
    return false;
}
void Joypad::setInterrupt(InterruptState* master){
    IS = master;
}
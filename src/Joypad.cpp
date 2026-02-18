#include "../include/Joypad.hpp"

void Joypad::update(){
    uint8_t currentButtonState = 0xFF;
    
    currentButtonState = (directions << 4) | buttons;
    
    uint8_t result = 0xC0 | (self.Joypad & 0xF0);
    if (!(self.Joypad & 0x20)) {
        result |= (buttons & 0x0F);
    }
    else if (!(self.Joypad & 0x10)) {
        result |= (directions & 0x0F);
    }
    
    self.Joypad = result;
    if (prevButtonState ^ currentButtonState) {
        IS->IF |= 0x10;
    }
    prevButtonState = currentButtonState;
}
JoypadState* Joypad::get(){
    return &self;
}
void Joypad::setInterrupt(InterruptState* master){
    IS = master;
}
#include "../include/Timer.hpp"

static const uint16_t frequency[4] = {1024, 16, 64, 256};

void Timer::step(int time){
    self.internalDIV += time;
    if (self.internalDIV >= 256){
        self.internalDIV -= 256;
        self.DIV = self.internalDIV >> 8;
    }

    if(self.TAC & 4){
        self.internalTIMA += time;
        int threshold = frequency[self.TAC & 3];
        while (self.internalTIMA >= threshold) {
            self.internalTIMA -= threshold;
            self.TIMA++;
            if (self.TIMA == 0) {
                self.TIMA = self.TMA;
                IS->IF |= TIMER;
            }
        }
    }
}
TimerState* Timer::get(){
    return &self;
}

void Timer::setInterrupt(InterruptState* master){
    IS = master;
}
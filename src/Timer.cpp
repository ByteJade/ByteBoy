#include "../include/Timer.hpp"

static const uint16_t frequency[4] = {1024, 16, 64, 256};

void Timer::step(int time){
    self.internalDIV += time;
    self.DIV = self.internalDIV >> 8;

    if(self.TAC & 4){
        self.internalTIMA += time;
        int threshold = frequency[self.TAC & 3];
        while (self.internalTIMA >= threshold) {
            self.internalTIMA -= threshold;
            self.TIMA++;
            if (self.TIMA == 0) {
                self.TIMA = self.TMA;
                IS.IF |= TIMER;
            }
        }
    }
}

bool Timer::write(uint16_t addr, uint8_t data){
    switch (addr) {
        case(0xFF04): // DIV
            self.DIV = 0;
            self.internalDIV = 0;
            return true;
        case(0xFF05): // TIMA
            self.TIMA = data;
            return true;
        case(0xFF06): // TMA
            self.TMA = data;
            return true;
        case(0xFF07): // TAC
            self.TAC = data & 7;
            return true;
    }
    return false;
}
bool Timer::read(uint16_t addr, uint8_t& data){
    switch (addr) {
        case(0xFF04): // DIV
            data = self.DIV;
            return true;
        case(0xFF05): // TIMA
            data = self.TIMA;
            return true;
        case(0xFF06): // TMA
            data = self.TMA;
            return true;
        case(0xFF07): // TAC
            data = self.TAC;
            return true;
    }
    return false;
}
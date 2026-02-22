#include "types.hpp"

struct TimerState{
    uint16_t internalDIV{0};
    uint16_t internalTIMA{0};
    uint8_t DIV{0};
    uint8_t TIMA{0};
    uint8_t TMA{0};
    uint8_t TAC{0};
};

class MemoryMaster;
class Timer{
    TimerState self;
    InterruptState* IS;
public:
    void step(int time);

    bool write(uint16_t addr, uint8_t data);
    bool read(uint16_t addr, uint8_t& data);

    void setInterrupt(InterruptState* master);
};
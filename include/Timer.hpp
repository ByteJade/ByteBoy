#include "types.hpp"

class MemoryMaster;
class Timer{
    TimerState self;
    InterruptState* IS;
public:
    void step(int time);

    TimerState* get();
    void setInterrupt(InterruptState* master);
};
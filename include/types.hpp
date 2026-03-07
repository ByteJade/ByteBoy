#pragma once
#include <cstdint>

#define SCW 160
#define SCH 144

enum INTERRUPTS {
    VBLANK = 0x1,
    STAT = 0x2,
    TIMER = 0x4,
    SERIAL = 0x8,
    INPUT = 0x10
};

struct InterruptState{
    uint8_t IE{0};
    uint8_t IF{0};
    uint8_t STAT{0};
};

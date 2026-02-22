#pragma once
#include <cstdint>

constexpr unsigned int SCW = 160;
constexpr unsigned int SCH = 144;

enum INTERRUPTS {
    VBLANK = 0x1,
    STAT = 0x2,
    TIMER = 0x4,
    SERIAL = 0x8,
    INPUT = 0x10
};
struct Vertex {
    float position[2];
    float texture[2];
};

struct Color{
    uint8_t r, g, b;
};
struct InterruptState{
    uint8_t IE{0};
    uint8_t IF{0};
    uint8_t STAT{0};
};
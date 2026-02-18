#pragma once
#include <cstdint>

constexpr unsigned int SCW = 160;
constexpr unsigned int SCH = 144;

struct Vertex {
    float position[2];
    float texture[2];
};

struct Color{
    uint8_t r, g, b;
};
struct TimerState{
    uint16_t internalDIV{0};
    uint16_t internalTIMA{0};
    uint8_t DIV{0};
    uint8_t TIMA{0};
    uint8_t TMA{0};
    uint8_t TAC{0};
};
struct PPUState{
    uint8_t LCDC{0};
    uint8_t SCY{0};
    uint8_t SCX{0};
    uint8_t LY{0};
    uint8_t LYC{0};
    uint8_t BGP{0};
    uint8_t OBP0{0};
    uint8_t OBP1{0};
    uint8_t WY{0};
    uint8_t WX{0};
    uint8_t OPRI{0};
    Color BGcolorBuffer[32];
    Color OBcolorBuffer[32];
};
struct InterruptState{
    uint8_t IE{0};
    uint8_t IF{0};
    uint8_t STAT{0};
};
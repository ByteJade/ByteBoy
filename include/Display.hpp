#pragma once

#include <SDL2/SDL.h>
#include "Joypad.hpp"

class MemoryMaster;
class Window{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* display;

    int _width;
    int _height;

    bool shouldClose;
public:
    Joypad joypad;
    
    Window(unsigned int width, unsigned int height, const char* name);
    ~Window();
    void poolEvents();
    bool poolFile(MemoryMaster& master);

    void drawLine(uint8_t* lines, uint8_t y);
    void setPixel(uint8_t x, int dy, uint32_t color);
    
    void show();
    bool isOpen();
};

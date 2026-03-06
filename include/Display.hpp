#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "Joypad.hpp"

class MemoryMaster;
class Window{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* display;

    bool shouldClose;

    int _width;
    int _height;
    void handleKey(SDL_Keycode key, bool isDown) ;
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
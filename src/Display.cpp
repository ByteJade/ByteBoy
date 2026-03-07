#include "../include/Display.hpp"
#include "../include/MEM.hpp"
#include <SDL2/SDL.h>

Window::Window(unsigned int width, unsigned int height, const char* name)
: _width(width), _height(height)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS  | SDL_INIT_AUDIO);
    
    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED);
    
    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCW, SCH);

    display = new uint32_t[SCW*SCH];
    for (unsigned int i = 0; i < SCW*SCH; i++)
        display[i] = 0xFFFFFFFF;
};
Window::~Window() {
    delete[] display;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::poolEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                shouldClose = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    _width = event.window.data1;
                    _height = event.window.data2;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case(SDLK_s):
                        joypad.directions &= ~0x8; break;
                    case(SDLK_w):
                        joypad.directions &= ~0x4; break;
                    case(SDLK_a):
                        joypad.directions &= ~0x2; break;
                    case(SDLK_d):
                        joypad.directions &= ~0x1; break;
                    case(SDLK_z):
                        joypad.buttons &= ~0x8; break;
                    case(SDLK_x):
                        joypad.buttons &= ~0x4; break;
                    case(SDLK_q):
                        joypad.buttons &= ~0x2; break;
                    case(SDLK_e):
                        joypad.buttons &= ~0x1; break;
                    case(SDLK_c):
                        joypad.buttons &= ~0xF; break;
                    default:
                        break;
                }
                joypad.update();
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case(SDLK_s):
                        joypad.directions |= 0x8; break;
                    case(SDLK_w):
                        joypad.directions |= 0x4; break;
                    case(SDLK_a):
                        joypad.directions |= 0x2; break;
                    case(SDLK_d):
                        joypad.directions |= 0x1; break;
                    case(SDLK_z):
                        joypad.buttons |= 0x8; break;
                    case(SDLK_x):
                        joypad.buttons |= 0x4; break;
                    case(SDLK_q):
                        joypad.buttons |= 0x2; break;
                    case(SDLK_e):
                        joypad.buttons |= 0x1; break;
                    case(SDLK_c):
                        joypad.buttons |= 0xF; break;
                    default:
                        break;
                }
                joypad.update();
                break;
        }
    }
}
bool Window::poolFile(MemoryMaster& master) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                shouldClose = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    _width = event.window.data1;
                    _height = event.window.data2;
                }
                break;
            case SDL_DROPFILE: {
                
                std::string filepath = event.drop.file;
                SDL_free(event.drop.file);
                if (master.readFromFile(filepath.c_str())) return true;
                
                break;
            }
        }
    }
    return false;
}
static constexpr int FRAME_DELAY = 1000 / 60;
void Window::show(){
    SDL_UpdateTexture(texture, NULL, display, SCW * sizeof(uint32_t));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // R, G, B, Alpha
    SDL_RenderClear(renderer);

    SDL_Rect dst = {0, 0, _width, _height};
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_RenderPresent(renderer);

    static Uint32 lastTime = SDL_GetTicks();

    Uint32 currentTime = SDL_GetTicks();
    Uint32 frameTime = currentTime - lastTime;
    
    if (frameTime < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frameTime);
    }
    
    lastTime = SDL_GetTicks(); 
}
const uint32_t col[4] = {0xFFFFFFFF, 0xAAAAAAFF, 0x555555FF, 0x000000FF};
void Window::drawLine(uint8_t* lines, uint8_t y){
    int dy = y * SCW;
    for (int x = 0; x < 160; x++) {
        int adr = dy + x;
        display[adr] = col[lines[x]];
    }
}
void Window::setPixel(uint8_t x, int dy, uint32_t color){
    int idx = dy + x;
    display[idx] = color;
}
bool Window::isOpen() { return !shouldClose; }
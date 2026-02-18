#include "types.hpp"

class MemoryMaster;
class Window;
class PPU{
    PPUState self;
    InterruptState* IS;

    MemoryMaster& MEM;
    Window& screen;

    uint16_t foundSprits[10];
    uint8_t founds = 0;
    uint8_t wline = 0;

    int MODE = 0;
    int timeCounter = 0;
    uint8_t BGlines[SCW];
    uint8_t OBlines[SCW];

    void update();
    void HBLANK();
    void VBLANK();
    void SEARCH();
    void DRAWING();
    
    void search();
    void drawing();
    void renderSprites();
    void render_BG_line();
    void render_window_line();
    
    void drawline(int& x, int dx, int dy, uint16_t tilemap);
    int cost();
public:
    PPU(MemoryMaster& master, Window& window);
    void step(int time);

    PPUState* get();
    void setInterrupt(InterruptState* master);
};
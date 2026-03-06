#include "types.hpp"
#include <cstdint>

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
    uint32_t BGcolorBuffer[32];
    uint32_t OBcolorBuffer[32];
};

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

    uint16_t BGP[32];
    uint16_t OBP[32];
    uint8_t BGsrc;
    uint8_t OBsrc;

    void updateColor(uint32_t& color, uint16_t data);

    void update();
    void setHBLANK();
    void setVBLANK();
    void setSEARCH();
    void setDRAWING();
    
    void search();
    void drawing();
    void renderSprites();
    void render_BG_line();
    void render_window_line();
    
    void drawline(int& x, int dx, int dy, uint16_t tilemap);
    int cost();

    void checkLYC();
public:
    PPU(MemoryMaster& master, Window& window);
    void step(int time);

    bool write(uint16_t addr, uint8_t data);
    bool read(uint16_t addr, uint8_t& data);
    
    void setInterrupt(InterruptState* master);
};
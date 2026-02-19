#include <algorithm>

#include "../include/PPU.hpp"
#include "../include/MEM.hpp"
#include "../include/Display.hpp"

PPU::PPU(MemoryMaster& master, Window& window) : MEM(master),
screen(window)
{
}
void PPU::update(){
    IS->STAT = (IS->STAT&0b11111100)|MODE;
    timeCounter = cost();
}
void PPU::setHBLANK(){
    if (IS->STAT & 0x08) IS->IF |= STAT;
    MODE = 0;
    update();
}
void PPU::setVBLANK(){
    IS->IF |= VBLANK;
    if (IS->STAT & 0x10) IS->IF |= STAT;
    MODE = 1;
    update();
}
void PPU::setSEARCH(){
    if (IS->STAT & 0x20) IS->IF |= STAT;
    MODE = 2;
    update();
}
void PPU::setDRAWING(){
    MODE = 3;
    update();
}

void PPU::search(){
    founds = 0;
    if (!(self.LCDC& 0x02)){
        return;
    }
    uint8_t sprite_height;

    if (self.LCDC&0x4) sprite_height = 16;
    else sprite_height = 8;
    for (uint16_t id = 0xFE00; id < 0xFEA0; id+=4){
        int16_t y = MEM.readOAM(id) - 16;
        if (self.LY >= y && self.LY < y + sprite_height){
            foundSprits[founds++] = id;
            if (founds == 10) return;
        }
    }

    // Sprite sorting by x coordinate. if coordinate the same - by address
    if (!MEM.isCGB){
        std::sort(foundSprits, foundSprits + founds,
        [this](uint16_t a, uint16_t b) {
            uint8_t x0 = this->MEM.readOAM(a+1);
            uint8_t x1 = this->MEM.readOAM(b+1);
            if (x0 != x1)
                return x0 > x1;
            return a > b;
        });
    }else if (!(self.OPRI&1)){
        std::sort(foundSprits, foundSprits + founds,
        [this](uint16_t a, uint16_t b) {
            return a > b;
        });
    }
}
void PPU::drawing(){
    if (self.LY >= SCH) return;
    bool isMaster = self.LCDC & 0x1;
    if (!isMaster && !MEM.isCGB){
        for (uint8_t x = 0; x < SCW; x++) BGlines[x] = 0;
    }else{
        render_BG_line();
    }
    if (self.LCDC & 0x20){
        render_window_line();
    }
    for (uint8_t x = 0; x < SCW; x++) OBlines[x] = 0x80;
    if (self.LCDC&0x2){
        renderSprites();
    }
    if (MEM.isCGB) {
        int dy = self.LY*SCW;
        for (uint8_t x = 0; x < SCW; x++){
            uint8_t Opx = OBlines[x];
            uint8_t Opd = Opx&0x3;
            uint8_t Bpx = BGlines[x];
            uint8_t Bpd = Bpx&0x3;
            uint8_t Bpp = Bpd+((Bpx>>2)&0x7)*4;

            if (Opd == 0){
                screen.setPixel(x, dy, self.BGcolorBuffer[Bpp]);
                continue;
            }
            uint8_t Opp = Opd+((Opx>>2)&0x7)*4;
            Color col;
            if(isMaster){
                if (Bpd == 0){
                    col = self.OBcolorBuffer[Opp];
                }else if ((Bpx & 0x80) | (Opx & 0x80)) {
                    col = self.BGcolorBuffer[Bpp];
                }else {
                    col = self.OBcolorBuffer[Opp];
                }
            }else{
                col = self.OBcolorBuffer[Opp];
            }
            screen.setPixel(x, dy, col);
        }
    }else{
        for (uint8_t x = 0; x < SCW; x++){
            uint8_t Opx = OBlines[x];
            uint8_t Bpx = BGlines[x];
            uint8_t Opd = Opx&0x3;
            uint8_t Bpd = Bpx&0x3;
            
            if (Bpx == 0){
                BGlines[x] = Opd;
            }else if (Opx&0x80){
                BGlines[x] = Bpd;
            }else BGlines[x] = Opd;
        }
        screen.drawLine(BGlines, self.LY);
    }
}
void PPU::renderSprites() {
    uint8_t sprite_height;
    if (self.LCDC & 0x4) sprite_height = 16;
    else sprite_height = 8;

    for (int i = 0; i < founds; i++){
        
        uint16_t addr = foundSprits[i];
        int oam_y = self.LY - (MEM.readOAM(addr) - 16);
        int oam_x = MEM.readOAM(addr+1) - 8;
        uint8_t tile_num = MEM.readOAM(addr+2);
        uint8_t flags = MEM.readOAM(addr+3);

        uint16_t tile_addr = 0x8000;

        uint8_t local_y = oam_y;

        uint8_t tile_line;
        if (sprite_height == 16) {
            tile_line = oam_y % 8;
        } else {
            tile_line = oam_y;
        }
        if (flags & 0x40) {  // Y flip
            local_y = sprite_height - 1 - local_y;
            tile_line = 7 - tile_line;
        }

        if (sprite_height == 16) {  // 8x16
            uint8_t tile_index = tile_num & 0xFE;
            
            if (local_y >= 8) {
                tile_index++;
                local_y -= 8;
            }
            tile_addr += tile_index * 16;
        } 
        else {  // 8x8
            tile_addr += tile_num * 16;
        }
        
        uint8_t lpalette = 0;
        uint8_t spritePalette = (flags&0x10) ? self.OBP1 : self.OBP0;
        if (MEM.isCGB){
            lpalette = (flags & 0x7) << 2;
        }
        
        uint16_t tile_line_offset = tile_addr + tile_line*2;
        uint8_t low_byte;
        uint8_t high_byte;
        if (flags & 0x08){
            low_byte = MEM.readVRAM1(tile_line_offset);
            high_byte = MEM.readVRAM1(tile_line_offset + 1);
        }else{
            low_byte = MEM.readVRAM0(tile_line_offset);
            high_byte = MEM.readVRAM0(tile_line_offset + 1);
        }
        
        uint8_t start_x = (oam_x < 0) ? static_cast<uint8_t>(-oam_x) : 0;
        bool xFlip = flags & 0x20;
        uint8_t meta = (flags & 0x80) | lpalette;
        int start_bit = xFlip ? 0 : 7;

        for (int x = start_x; x < 8; x++) {
            int pos = oam_x + x;
            if (pos >= 160) break;
            
            int bit = start_bit + (xFlip ? x : -x);
            uint8_t color = ((high_byte >> bit) & 1) << 1 |
                            ((low_byte >> bit) & 1);
            
            if (color == 0) continue;
            
            uint8_t final_color;
            if (MEM.isCGB){
                final_color = color;
            } else{
                final_color = (spritePalette >> (color * 2)) & 3;
            }
            OBlines[pos] = final_color | meta;
        }
    }
}
void PPU::render_BG_line() {
    uint16_t tilemap_addr = (self.LCDC & 0x8) ? 0x9C00 : 0x9800;
    
    uint8_t scy = self.SCY;
    uint8_t scx = self.SCX;
    
    uint8_t bg_y = (self.LY + scy);
    for (int x = 0; x < 160;) {
        uint8_t bg_x = (x + scx);

        drawline(x, bg_x, bg_y, tilemap_addr);
    }
}
void PPU::render_window_line() {
    int wx = (int)self.WX - 7;
    if (self.LY < self.WY || wx >= 160) {
        return;
    }
    uint16_t tilemap_addr = (self.LCDC & 0x40) ? 0x9C00 : 0x9800;
    uint8_t start_x = wx * (wx >= 0);
    
    for (int x = start_x; x < 160;) {
        uint8_t win_x = x - start_x;
        
        drawline(x, win_x, wline, tilemap_addr);
    }
    wline++;
}
void PPU::drawline(int& x, int dx, int dy, uint16_t tilemap){
    uint8_t tile_x = dx / 8;
    uint8_t tile_y = dy / 8;
    
    uint16_t tilemap_index = tilemap + tile_y * 32 + tile_x;
    uint8_t tile_num = MEM.readVRAM0(tilemap_index);

    uint16_t tile_addr;
    if (self.LCDC & 0x10) {
        tile_addr = 0x8000 + (uint16_t(tile_num)*16);
    } else {
        tile_addr = 0x9000 + (int8_t(tile_num)*16);
    }
    
    uint8_t pixel_x_in_tile = dx % 8;
    uint8_t pixel_y_in_tile = dy % 8;

    uint8_t flags = 0;
    uint8_t lpalette = 0;
    uint8_t palette = self.BGP;
    if (MEM.isCGB){
        flags = MEM.readVRAM1(tilemap_index);
        lpalette = (flags & 0x07) << 2;

        if (flags & 0x40) {  // Y flip
            pixel_y_in_tile = 7 - pixel_y_in_tile;
        }
    }

    uint16_t tile_line_offset = tile_addr + (pixel_y_in_tile * 2);

    uint8_t low_byte;
    uint8_t high_byte;
    if ((flags & 0x08)){
        low_byte = MEM.readVRAM1(tile_line_offset);
        high_byte = MEM.readVRAM1(tile_line_offset + 1);
    }else{
        low_byte = MEM.readVRAM0(tile_line_offset);
        high_byte = MEM.readVRAM0(tile_line_offset + 1);
    }
    
    bool xFlip = flags & 0x20;
    uint8_t meta = (flags & 0x80) | lpalette;
    for (int n = pixel_x_in_tile; n < 8; n++){
        if (x >= 160) return;
        uint8_t bit;
        if (xFlip) {
            bit = n;
        } else {
            bit = 7 - n;
        }
        uint8_t color = ((high_byte >> bit) & 1) << 1 |
                        ((low_byte >> bit) & 1);
        
        uint8_t final_color;
        if (MEM.isCGB){
            final_color = color;
        }else {
            final_color = (palette >> (color * 2)) & 3;
        }
        BGlines[x++] = final_color | meta;
    }
}

int PPU::cost(){
    switch (MODE) {
        case 0:
            return 204;
        case 1:
            return 456;
        case 2:
            return 80;
        case 3:
            return 172;
    }
    return 0;
}
void PPU::step(int time){
    if ( !(self.LCDC >> 7) )
    {
        self.LY = 0;
        wline = 0;
        MODE = 0;
        IS->STAT &= 0b11111100;
        return;
    }
    timeCounter -= time;
    if (timeCounter <= 0){
        timeCounter += cost();
        switch (MODE) {
            case 0:
                self.LY++;
                MEM.checkLYC();
                if (self.LY >= SCH) { setVBLANK(); }
                else setSEARCH();
                MEM.HDMAstep();
                break;
            case 1:
                self.LY++;
                MEM.checkLYC();
                if (self.LY > 153){
                    self.LY = 0;
                    wline = 0;
                    screen.show();
                    setSEARCH();
                }
                break;
            case 2:
                search();
                setDRAWING();
                break;
            case 3:
                drawing();
                setHBLANK();
                screen.poolEvents();
                break;
        }
    }
}

PPUState* PPU::get(){
    return &self;
}

void PPU::setInterrupt(InterruptState* master){
    IS = master;
}

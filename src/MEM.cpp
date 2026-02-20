#include <fstream>
#include <iostream>

#include "../include/MEM.hpp"
#include "../include/Display.hpp"

void extractFilename(std::string& path) {
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        path = path.substr(0, dot_pos);
    }
}

MemoryMaster::MemoryMaster(Joypad& joy) : joypad(joy){
    OAM = new uint8_t[0xA0];
    IO = new uint8_t[0x100];
}
MemoryMaster::~MemoryMaster(){
    if (CRAMsize != 0){
        std::cout <<"saved\n";
        writeSaveToFile();
    }
    if (ROM) delete[] ROM;
    if (CRAM) delete[] CRAM;
    if (VRAM) delete[] VRAM;
    if (RAM) delete[] RAM;
    delete[] OAM;
    delete[] IO;
}
void MemoryMaster::handleMBC1(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        if (CRAMsize == 0) return;
        CRAMenable = (data&0x0F) == 0x0A;
    }else if (addr < 0x4000){
        updateROMoffset((ROMbank & 0x60) | (data  & 0x1F));
    }else if (addr < 0x6000){
        uint8_t bank = data & 0x03;
        if (bankingMode) {
           updateRAMoffset(bank);
        } else {
            updateROMoffset((ROMbank & 0x1F) | (bank << 5));
        }
    }else{
        bankingMode = data & 0x01;
        if (!bankingMode) RAMoffset = 0;
    }
}
void MemoryMaster::handleMBC2(uint16_t addr, uint8_t data){
    if (addr & 0x0100){
        if (addr < 0x4000){
            updateROMoffset(data & 0xF);
        }
    }else if (addr < 0x2000){
        if (CRAMsize == 0) return;
        CRAMenable = (data&0x0F) == 0xA;
    }
}
void MemoryMaster::handleMBC3(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        if (CRAMsize == 0) return;
        CRAMenable = (data&0xF) == 0x0A;
    }else if (addr < 0x4000){
        updateROMoffset(data & 0x7F);
    }else if (addr < 0x6000){
        updateRAMoffset(data & 0xF);
    }//else{
        // RTC
    //}
}
// idk why, but only MBC5 works correctly
void MemoryMaster::handleMBC5(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        if (CRAMsize == 0) return;
        CRAMenable = ((data&0x0F) == 0x0A);
    }else if (addr < 0x3000){
        updateROMoffset((ROMbank & 0x100) | data);
    }else if (addr < 0x4000){
        updateROMoffset((ROMbank & 0xFF) | ((data & 1) << 8));
    }else if (addr < 0x6000){
        updateRAMoffset(data & 0xF);
    }//else{
        // RTC
    //}
}
void MemoryMaster::updateROMoffset(uint16_t data){
    ROMbank = data;

    if (MBCtype != MBC2) ROMbank &= (totalROMbanks - 1);
    else ROMbank %= (totalROMbanks);
    if (ROMbank == 0 && MBCtype != MBC5) ROMbank = 1;
    ROMoffset = (ROMbank-1) * ROM_BANKSIZE;
}
void MemoryMaster::updateRAMoffset(uint16_t data){
    RAMoffset = data;
    RAMoffset &= (totalRAMbanks - 1);
    RAMoffset *=  CRAM_BANKSIZE;
}
void MemoryMaster::updateColor(Color& color, uint16_t data){
    uint8_t r = (data & 0x1F);
    uint8_t g = (data >> 5) & 0x1F;
    uint8_t b = (data >> 10) & 0x1F;
    color.r = (r << 3) | (r >> 2);
    color.g = (g << 3) | (g >> 2);
    color.b = (b << 3) | (b >> 2);
}
void MemoryMaster::HDMAstep(){
    if (!hdma.work) return;

    for (uint16_t x = 0; x < 0x10; x++){
        uint8_t byte = read(hdma.src++);
        writeVRAM(hdma.dst++, byte);
    }
    hdma.hdma5--;
    if (hdma.hdma5 == 0xFF) {
        hdma.work = false;
    }
}
static uint8_t fail = 0xFF;
uint8_t& MemoryMaster::read(uint16_t addr){
    if (addr < 0x4000){
        return ROM[addr];
    }else if (addr < 0x8000){
        return ROM[ROMoffset + addr];
    }else if (addr < 0xA000){
        return readVRAM(addr);
    }else if (addr < 0xC000){
        if (CRAMenable){
            return CRAM[RAMoffset + addr - 0xA000];
        }
        return fail;
    }else if (addr < 0xE000){
        return readWRAM(addr);
    }else{
        if (addr < 0xFE00){
            return readWRAM(addr-0x2000);
        }if (addr < 0xFEA0){
            return readOAM(addr);
        }if (addr >= 0xFF00){
            return readIO(addr);
        } 
    }
    return fail;
}
uint8_t& MemoryMaster::readWRAM(uint16_t addr){
    if (addr < 0xD000){
        return RAM[addr - 0xC000];
    }else{
        uint16_t offset = WRAMbank*WRAM_BANKSIZE + addr - 0xD000;
        return RAM[offset];
    }
}
uint8_t MemoryMaster::readVRAM0(uint16_t addr){
    return VRAM[addr - 0x8000];
}
uint8_t MemoryMaster::readVRAM1(uint16_t addr){
    return VRAM[addr - 0x6000];
}
uint8_t& MemoryMaster::readVRAM(uint16_t addr){
    return VRAM[VRAMbank*VRAM_BANKSIZE + addr - 0x8000];
}
uint8_t& MemoryMaster::readOAM(uint16_t addr){
    return OAM[addr-0xFE00];
}
uint8_t& MemoryMaster::readIO(uint16_t addr){
    switch (addr) {
        case(0xFF00): // Joypad
            return JS->Joypad;
        case(0xFF04): // DIV
            return TS->DIV;
        case(0xFF05): // TIMA
            return TS->TIMA;
        case(0xFF06): // TMA
            return TS->TMA;
        case(0xFF07): // TAC
            return TS->TAC;
        case(0xFF0F): // IF
            return IS->IF;
        case(0xFF40): // LCDC
            return PS->LCDC;
        case(0xFF41): // STAT
            return IS->STAT;
        case(0xFF42): // SCY
            return PS->SCY;
        case(0xFF43): // SCX
            return PS->SCX;
        case(0xFF44): // LY
            return PS->LY;
        case(0xFF45): // LYC
            return PS->LYC;
        case(0xFF46): // DMA
            return fail;
        case(0xFF47): // BGP
            return PS->BGP;
        case(0xFF48): // OBP0
            return PS->OBP0;
        case(0xFF49): // OBP1
            return PS->OBP1;
        case(0xFF51): // HDMA1
            if (isCGB) return hdma.src_hight;
            break;
        case(0xFF52): // HDMA2
            if (isCGB) return hdma.src_low;
            break;
        case(0xFF53): // HDMA3
            if (isCGB) return hdma.dst_hight;
            break;
        case(0xFF54): // HDMA4
            if (isCGB) return hdma.dst_low;
            break;
        case (0xFF55):
            if (isCGB) return hdma.hdma5;
            break;
        case(0xFF4A): // WY
            return PS->WY;
        case(0xFF4B): // WX
            return PS->WX;
        case (0xFF4F):
            if (isCGB) return VRAMbank;
            break;
        case(0xFF6C): // OPRI
            if (isCGB) return PS->OPRI;
            break;
        case(0xFF70): // WBK
            if (isCGB) return WRAMbank;
            break;
        case(0xFFFF): // WX
            return IS->IE;
    }
    return IO[addr-0xFF00];
}
void MemoryMaster::write(uint16_t addr, uint8_t data){
    if (addr < 0x8000){
        switch (MBCtype) {
            case (MBC1):{
                handleMBC1(addr, data);
            }break;
            case (MBC2):{
                handleMBC2(addr, data);
            }break;
            case (MBC3):{
                handleMBC3(addr, data);
            }break;
            case (MBC5):{
                handleMBC5(addr, data);
            }break;
        }
    }else if (addr < 0xA000){
        writeVRAM(addr, data);
    }else if (addr < 0xC000){
        if (CRAMenable){
            CRAM[uint32_t(addr-0xA000) + RAMoffset] = data;
        }
    }else if (addr < 0xE000){
        writeWRAM(addr, data);
    }else{
        if (addr < 0xFE00){
            writeWRAM(addr - 0x2000, data);
        }else if (addr < 0xFEA0){
            writeOAM(addr, data);
        }else if (addr >= 0xFF00){
            writeIO(addr, data);
        }
    }
}
void MemoryMaster::writeVRAM(uint16_t addr, uint8_t data){
    VRAM[VRAMbank*VRAM_BANKSIZE + addr - 0x8000] = data;
}
void MemoryMaster::writeWRAM(uint16_t addr, uint8_t data){
    if (addr < 0xD000){
        RAM[addr - 0xC000] = data;
    }else{
        uint16_t offset = WRAMbank*WRAM_BANKSIZE + addr - 0xD000;
        RAM[offset] = data;
    }
}
void MemoryMaster::writeOAM(uint16_t addr, uint8_t data){
    OAM[addr-0xFE00] = data;
}
void MemoryMaster::writeIO(uint16_t addr, uint8_t data){
    switch (addr) {
        case(0xFF00): // Joypad
            JS->Joypad = (data&0xF0) | (JS->Joypad&0xF);
            joypad.update();
            break;
        case(0xFF04): // DIV
            TS->DIV = 0;
            TS->internalDIV = 0;
            break;
        case(0xFF05): // TIMA
            TS->TIMA = data;
            break;
        case(0xFF06): // TMA
            TS->TMA = data;
            break;
        case(0xFF07): // TAC
            TS->TAC = data & 7;
            break;
        case(0xFF0F): // IF
            IS->IF = data;
            break;
        case(0xFF40): // LCDC
            PS->LCDC = data;
            break;
        case(0xFF41): // STAT
            IS->STAT = (IS->STAT & 0x87) | (data & 0x78);
            break;
        case(0xFF42): // SCY
            PS->SCY = data;
            break;
        case(0xFF43): // SCX
            PS->SCX = data;
            break;
        case(0xFF44): // LY
            break;
        case(0xFF45): // LYC
            PS->LYC = data;
            checkLYC();
            break;
        case (0xFF46):{
            uint16_t value = data << 8;
            for (int x = 0; x < 160; x++){
                OAM[x] = read(value+x);
            } break;
        }
        case(0xFF47): // BGP
            PS->BGP = data;
            break;
        case(0xFF48): // OBP0
            PS->OBP0 = data;
            break;
        case(0xFF49): // OBP1
            PS->OBP1 = data;
            break;
        case(0xFF4A): // WY
            PS->WY = data;
            break;
        case(0xFF4B): // WX
            PS->WX = data;
            break;
        case (0xFF4F):
            if (isCGB){
                VRAMbank = data & 1;
            } break;
        case(0xFF51): // HDMA1
            if (isCGB) hdma.src_hight = data;
            break;
        case(0xFF52): // HDMA2
            if (isCGB) hdma.src_low = data;
            break;
        case(0xFF53): // HDMA3
            if (isCGB) hdma.dst_hight = data;
            break;
        case(0xFF54): // HDMA4
            if (isCGB) hdma.dst_low = data;
            break;
        case (0xFF55):
            if (isCGB){
                bool mode_hblank = (data & 0x80) ? 1 : 0;

                if (hdma.work && !mode_hblank) {
                    // Cancel ongoing H-Blank HDMA transfer
                    hdma.work = false;
                    hdma.hdma5 = 0xff;
                    return;
                }
                uint16_t blocks = (data & 0x7F) + 1;
                hdma.src = (hdma.src_hight << 8) | (hdma.src_low & 0xF0);
                hdma.dst = (hdma.dst_hight << 8) | (hdma.dst_low & 0xF0);
                hdma.dst = (hdma.dst & 0x1fff) | 0x8000;
                
                if (!mode_hblank){
                    uint16_t len = blocks * 16;
                    for (uint16_t i = 0; i < len; i++){
                        uint8_t bit = read(hdma.src++);
                        writeVRAM(hdma.dst++, bit);
                    }
                    hdma.hdma5 = 0xFF;
                }else{
                    hdma.work = true;
                    hdma.hdma5 = blocks - 1;
                    if ((IS->STAT & 3) == 0) // H-Blank
                        HDMAstep();
                }
            } break;
        case (0xFF69):
            if (isCGB){
                uint8_t& reg = readIO(0xFF68);
                uint8_t id = (reg&0x3F)/2;
                if (reg%2){
                    BGP[id] |= data << 8;
                }else{
                    BGP[id] = data;
                }
                updateColor(PS->BGcolorBuffer[id], BGP[id]);
                if (reg&0x80) reg++;
            } break;
        case (0xFF6B):
            if (isCGB){
                uint8_t& reg = readIO(0xFF6A);
                uint8_t id = (reg&0x3F)/2;
                if (reg%2){
                    OBP[id] |= data << 8;
                }else{
                    OBP[id] = data;
                }
                updateColor(PS->OBcolorBuffer[id], OBP[id]);
                if (reg&0x80) reg++;
            } break;
        case(0xFF6C): // OPRI
            if (isCGB) PS->OPRI = data & 1;
            break;
        case (0xFF70):
            if (isCGB){
                WRAMbank = data & 0x07;
                if (WRAMbank == 0) WRAMbank = 1;
            } break;
        case(0xFFFF): // LCDC
            IS->IE = data;
            break;
        default:
            if (addr >= 0xFF00) IO[addr-0xFF00] = data;
    }
}
void MemoryMaster::readSaveFromFile(){
    std::ifstream file(readedFilename+".sv", std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening save\n";
        return;
    }
    file.read(reinterpret_cast<char*>(CRAM), CRAMsize);
    file.close();
}
void MemoryMaster::writeSaveToFile(){
    std::ofstream file(readedFilename+".sv", std::ios::binary);
    if (!file) {
        std::cerr << "Save error\n";
        return;
    }
    file.write(reinterpret_cast<char*>(CRAM), sizeof(char) * CRAMsize);
    file.close();
}
bool MemoryMaster::readFromFile(const char* filename){
    std::ifstream file(filename, std::ios::binary);
    readedFilename = filename;
    extractFilename(readedFilename);
    std::cout<<"read: "<<filename<<"\n";
    if (!file.is_open()) {
        std::cerr << "Error opening file\n";
        return false;
    }
    char byte;

    file.seekg(0x143, std::ios::beg);
    file.get(byte);
    uint8_t GBtype = byte;
    if (GBtype == 0xC0 || GBtype == 0x80){
        isCGB = true;
        std::cout<<"CGB mode\n";
        VRAM = new uint8_t[0x4000]; // 2 banks
        RAM = new uint8_t[0x10000]; // 8 banks
        for (int x = 0; x < 0x4000; x++) VRAM[x] = 0;
    }else{
        std::cout<<"DMG mode\n";
        VRAM = new uint8_t[0x2000];
        RAM = new uint8_t[0x2000];
        for (int x = 0; x < 0x2000; x++) VRAM[x] = 0;
    }

    file.seekg(0x0147, std::ios::beg);
    file.get(byte);
    if (byte <= 0x03) MBCtype = MBC1;
    else if (byte <= 0x0D) MBCtype = MBC2;
    else if (byte <= 0x13) MBCtype = MBC3;
    else MBCtype = MBC5;
    std::cout<<"MCB: "<<MBCtype<<"\n";

    file.seekg(0x0148, std::ios::beg);
    file.get(byte);
    switch (byte) {
        case(0x00): ROMsize = 32 * 1024; break;
        case(0x01): ROMsize = 64 * 1024; break;
        case(0x02): ROMsize = 128 * 1024; break;
        case(0x03): ROMsize = 256 * 1024; break;
        case(0x04): ROMsize = 512 * 1024; break;
        case(0x05): ROMsize = 1024 * 1024; break;
        case(0x06): ROMsize = 2048 * 1024; break;
        case(0x07): ROMsize = 4096 * 1024; break;
        case(0x08): ROMsize = 8192 * 1024; break;
        default:
            std::cout<<"unrecognizer ROM\n";
            return false;
    }
    totalROMbanks = ROMsize / (16*1024);
    ROM = new uint8_t[ROMsize];
    std::cout<<"ROM banks: "<<int(totalROMbanks)<<"\n";

    file.seekg(0x0149, std::ios::beg);
    file.get(byte);
    switch (byte) {
        case(0x00): CRAMsize = 0; break;
        case(0x01): CRAMsize = 2 * 1024; break;
        case(0x02): CRAMsize = 8 * 1024; break;
        case(0x03): CRAMsize = 32 * 1024; break;
        case(0x04): CRAMsize = 128 * 1024; break;
        case(0x05): CRAMsize = 64 * 1024; break;
        case(0x06): CRAMsize = 128 * 1024; break;
        case(0x07): CRAMsize = 256 * 1024; break;
        case(0x08): CRAMsize = 1024 * 1024; break;
        default:
            std::cout<<"unrecognizer RAM\n";
            return false;
    }
    if (CRAMsize != 0){
        CRAM = new uint8_t[CRAMsize];
        readSaveFromFile();
    }
    totalRAMbanks = CRAMsize / (8 * 1024);
    std::cout<<"RAM banks: "<<int(totalRAMbanks)<<"\n";

    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(ROM), ROMsize)) {
        return false;
    }
    file.close();


    return true;
}
void MemoryMaster::setTimer(TimerState* master){
    TS = master;
}
void MemoryMaster::setJoypad(JoypadState* master){
    JS = master;
}
void MemoryMaster::setPPU(PPUState* master){
    PS = master;
}
void MemoryMaster::setInterrupt(InterruptState* master){
    IS = master;
}

void MemoryMaster::checkLYC(){
    if (PS->LY == PS->LYC){
        IS->STAT |= 0x04;
        if (IS->STAT & 0x40)
            IS->IF |= STAT;
    }else {
        IS->STAT &= ~0x04;
    }
}

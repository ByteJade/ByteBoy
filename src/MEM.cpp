#include <cstdint>
#include <fstream>
#include <iostream>

#include "../include/MEM.hpp"
#include "../include/APU.hpp"
#include "../include/PPU.hpp"
#include "../include/Timer.hpp"
#include "../include/Display.hpp"

void extractFilename(std::string& path) {
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        path = path.substr(0, dot_pos);
    }
}

MemoryMaster::MemoryMaster(){
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
// idk how, but this is work
void MemoryMaster::handleMBC1(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        CRAMenable = (data&0xF) == 0xA;
    }else if (addr < 0x4000){
        uint8_t bank = data & 0x1F;
        bank += bank == 0;
        if (bankingMode) updateROMoffset(bank);
        else updateROMoffset((ROMbank & 0x60) | bank);
    }else if (addr < 0x6000){
        uint8_t bank = data & 3;
        uint8_t bankHight = bank << 5;
        updateROMoffset((ROMbank & 0x1F) | bankHight);
        if (bankingMode) {
            updateRAMoffset(bank);
            uint16_t ROM0bank = bankHight & (totalROMbanks - 1);
            ROM0offset = ROM0bank * ROM_BANKSIZE;
        }
    }else{
        bankingMode = data & 1;
        if (!bankingMode){
            ROM0offset = 0;
            RAMoffset = 0;
        }
    }
}
void MemoryMaster::handleMBC2(uint16_t addr, uint8_t data){
    if (addr & 0x0100){
        if (addr < 0x4000){
            uint8_t bank = data & 0xF;
            bank += bank == 0;
            updateROMoffset(bank);
        }
    }else if (addr < 0x2000){
        CRAMenable = (data&0xF) == 0xA;
    }
}
void MemoryMaster::handleMBC3(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        CRAMenable = (data&0xF) == 0x0A;
    }else if (addr < 0x4000){
        updateROMoffset(data & 0x7F);
    }else if (addr < 0x6000){
        updateRAMoffset(data & 0xF);
    }//else{
        // RTC
    //}
}
void MemoryMaster::handleMBC5(uint16_t addr, uint8_t data){
    if (addr < 0x2000){
        CRAMenable = ((data&0xF) == 0xA);
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
    data &= (totalROMbanks - 1);
    ROMbank = data;
    ROM1offset = (ROMbank-1) * ROM_BANKSIZE;
}
void MemoryMaster::updateRAMoffset(uint16_t data){
    data &= (totalRAMbanks - 1);
    RAMoffset = data;
    RAMoffset *=  CRAM_BANKSIZE;
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
uint8_t MemoryMaster::read(uint16_t addr){
    if (addr < 0x4000){
        return ROM[ROM0offset + addr];
    }else if (addr < 0x8000){
        return ROM[ROM1offset + addr];
    }else if (addr < 0xA000){
        return readVRAM(addr);
    }else if (addr < 0xC000){
        if (CRAMenable){
            return CRAM[RAMoffset + addr - 0xA000];
        }
        return 0xFF;
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
    return 0xFF;
}
uint8_t MemoryMaster::readWRAM(uint16_t addr){
    if (addr < 0xD000){
        return RAM[addr - 0xC000];
    }else{
        uint16_t offset = WRAMoffset + addr - 0xD000;
        return RAM[offset];
    }
}
uint8_t MemoryMaster::readVRAM0(uint16_t addr){
    return VRAM[addr - 0x8000];
}
uint8_t MemoryMaster::readVRAM1(uint16_t addr){
    return VRAM[addr - 0x6000];
}
uint8_t MemoryMaster::readVRAM(uint16_t addr){
    return VRAM[VRAMoffset + addr - 0x8000];
}
uint8_t MemoryMaster::readOAM(uint16_t addr){
    return OAM[addr-0xFE00];
}
uint8_t MemoryMaster::readIO(uint16_t addr){
    uint8_t data = 0xFF;
    if (ppu->read(addr, data)) return data;
    if (timer->read(addr, data)) return data;
    if (joypad->read(addr, data)) return data;
    if (apu->read(addr, data)) return data;
    switch (addr) {
        case(0xFF0F): // IF
            return IS->IF;
        case(0xFF41): // STAT
            return IS->STAT;
        case(0xFF46): // DMA
            return 0xFF;
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
        case (0xFF4F):
            if (isCGB) return VRAMbank;
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
    VRAM[VRAMoffset + addr - 0x8000] = data;
}
void MemoryMaster::writeWRAM(uint16_t addr, uint8_t data){
    if (addr < 0xD000){
        RAM[addr - 0xC000] = data;
    }else{
        uint16_t offset = WRAMoffset + addr - 0xD000;
        RAM[offset] = data;
    }
}
void MemoryMaster::writeOAM(uint16_t addr, uint8_t data){
    OAM[addr-0xFE00] = data;
}
void MemoryMaster::writeIO(uint16_t addr, uint8_t data){
    if (ppu->write(addr, data)) return;
    if (timer->write(addr, data)) return;
    if (joypad->write(addr, data)) return;
    if (apu->write(addr, data)) return;
    switch (addr) {
        case(0xFF0F): // IF
            IS->IF = data;
            break;
        case(0xFF41): // STAT
            IS->STAT = (IS->STAT & 0x87) | (data & 0x78);
            break;
        case (0xFF46):{
            uint16_t value = data << 8;
            for (int x = 0; x < 160; x++){
                OAM[x] = read(value+x);
            } break;
        }
        case (0xFF4F):
            if (isCGB){
                VRAMbank = data & 1;
                VRAMoffset = VRAMbank * VRAM_BANKSIZE;
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
        case (0xFF70):
            if (isCGB){
                WRAMbank = data & 0x07;
                if (WRAMbank == 0) WRAMbank = 1;
                WRAMoffset = WRAMbank * WRAM_BANKSIZE;
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
        case(0x00): CRAMsize = 2 * 1024; break; // 0
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
void MemoryMaster::setTimer(Timer* master){
    timer = master;
}
void MemoryMaster::setJoypad(Joypad* master){
    joypad = master;
}
void MemoryMaster::setPPU(PPU* master){
    ppu = master;
}
void MemoryMaster::setAPU(APU* master){
    apu = master;
}
void MemoryMaster::setInterrupt(InterruptState* master){
    IS = master;
}

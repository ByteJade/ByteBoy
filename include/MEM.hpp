#pragma once

#include <string>
#include "Joypad.hpp"
#include "types.hpp"

static const unsigned ROM_BANKSIZE      = 0x4000; /* 16K */
static const unsigned WRAM_BANKSIZE     = 0x1000; /* 4K */
static const unsigned VRAM_BANKSIZE     = 0x2000; /* 8K */
static const unsigned CRAM_BANKSIZE   = 0x2000; /* 8K */

enum MBC{
    MBC1 = 1,
    MBC2 = 2,
    MBC3 = 3,
    MBC5 = 5
};

struct HDMAstate{
    uint8_t src_low;
    uint8_t src_hight;
    uint8_t dst_low;
    uint8_t dst_hight;
    uint8_t hdma5{false};

    uint16_t src;
    uint16_t dst;
    uint16_t len;
    bool work;
};

class MemoryMaster{
    InterruptState* IS;
    TimerState* TS;
    PPUState* PS;
    JoypadState* JS;
    Joypad& joypad;

    uint8_t* ROM{nullptr};
    uint8_t* CRAM{nullptr};
    uint8_t* RAM{nullptr};
    uint8_t* VRAM{nullptr};
    uint8_t* OAM;
    uint8_t* IO;
    uint16_t totalROMbanks = 1;
    uint16_t totalRAMbanks = 0;

    int32_t ROMoffset = 0;
    uint32_t RAMoffset = 0;

    int16_t ROMbank = 1;
    uint8_t WRAMbank = 1;
    uint16_t RAMbank = 0;
    uint8_t VRAMbank = 0;

    uint32_t CRAMsize = 0;
    bool CRAMenable = false;
    uint32_t ROMsize = 0;

    bool bankingMode = 0;
    MBC MBCtype;

    void handleBanking(uint16_t addr, uint8_t data);
    void handleMBC1(uint16_t addr, uint8_t data);
    void handleMBC2(uint16_t addr, uint8_t data);
    void handleMBC3(uint16_t addr, uint8_t data);
    void handleMBC5(uint16_t addr, uint8_t data);
    void updateROMoffset(uint16_t data);
    void updateRAMoffset(uint16_t data);
    void readSaveFromFile();
    void writeSaveToFile();
    std::string readedFilename;
    uint16_t BGP[32];
    uint16_t OBP[32];
    uint8_t BGsrc;
    uint8_t OBsrc;

    void updateColor(Color& color, uint16_t data);
    
    HDMAstate hdma;
public:
    bool isCGB = false;
    bool doubleCPUspeed = false;

    MemoryMaster(Joypad& joy);
    ~MemoryMaster();
    uint8_t& read(uint16_t addr);
    uint8_t& readWRAM(uint16_t addr);
    uint8_t readVRAM0(uint16_t addr);
    uint8_t readVRAM1(uint16_t addr);
    uint8_t& readVRAM(uint16_t addr);
    uint8_t& readOAM(uint16_t addr);
    uint8_t& readIO(uint16_t addr);

    void write(uint16_t addr, uint8_t data);
    void writeWRAM(uint16_t addr, uint8_t data);
    void writeVRAM(uint16_t addr, uint8_t data);
    void writeOAM(uint16_t addr, uint8_t data);
    void writeIO(uint16_t addr, uint8_t data);
    void HDMAstep();
    bool readFromFile(const char* filename);

    void setTimer(TimerState* master);
    void setJoypad(JoypadState* master);
    void setPPU(PPUState* master);
    void setInterrupt(InterruptState* master);

    void checkLYC();
};

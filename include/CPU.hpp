#include "types.hpp"

enum Flags {
    car = 0x10,
    hcar = 0x20,
    sub = 0x40, 
    zero = 0x80
};
class MemoryMaster;
class CPU{
    MemoryMaster& MEM;
    InterruptState* IS;

    bool halt{false};
    bool ime{false};
    uint8_t extraTime{0};
    
    uint8_t A{0x11};
    uint8_t B{0x00};
    uint8_t C{0x00};
    uint8_t D{0xFF};
    uint8_t E{0x56};
    uint8_t H{0x00};
    uint8_t L{0x0D};
    uint8_t F{zero};
    uint16_t PC{0x100};
    uint16_t SP{0xFFFE};

    uint16_t nn();
    uint8_t n();

    uint16_t AF();
    uint16_t BC();
    uint16_t DE();
    uint16_t HL();

    void SETAF(uint16_t n);
    void SETBC(uint16_t n);
    void SETDE(uint16_t n);
    void SETHL(uint16_t n);

    uint8_t POP();
    uint16_t POP16();
    void SAVESP();
    void PUSH(uint16_t nn);

    uint8_t readTableR(uint8_t code);
    uint16_t readTableRP(uint8_t code);
    uint16_t readTableRP2(uint8_t code);

    void writeTableR(uint8_t code, uint8_t data);
    void writeTableRP(uint8_t code, uint16_t data);
    void writeTableRP2(uint8_t code, uint16_t data);

    bool readTableCC(uint8_t code);
    void executeTableACC(uint8_t code);
    void executeTableRJAO(uint8_t code);
    void executeTableCRAO(uint8_t code);
    void executeTableCJAO(uint8_t code);
    void executeTableALU(uint8_t code, uint8_t data);
    uint8_t executeTableROT(uint8_t code, uint8_t data);
// Exec opcodes
    int execute(uint8_t opcode);
    int executeCB(uint8_t opcode);
// math ops
    void ADD8(uint8_t b);
    void ADC8(uint8_t b);
    void SUB8(uint8_t b);
    void SBC8(uint8_t b);
    void AND8(uint8_t b);
    void OR8(uint8_t b);
    void XOR8(uint8_t b);
    void CP8(uint8_t b);

    uint8_t INC8(uint8_t b);
    uint8_t DEC8(uint8_t b);

    uint16_t ADD16(uint16_t a, uint16_t b);
    uint16_t ADD16S(uint16_t nn, int8_t e);
    
    void CALL();
    
    void RLCA();
    void RRCA();
    void RLA();
    void RRA();
    void DAA();
    void CPL();
    void SCF();
    void CCF();
    void STOP();

    uint8_t SRL(uint8_t reg);
    uint8_t SLA(uint8_t reg);
    uint8_t SRA(uint8_t reg);
    uint8_t RR(uint8_t reg);
    uint8_t RL(uint8_t reg);
    uint8_t RLC(uint8_t reg);
    uint8_t RRC(uint8_t reg);
    uint8_t SWAP(uint8_t reg);

    void BIT(uint8_t bit, uint8_t value);
    uint8_t SET(uint8_t bit, uint8_t value);
    uint8_t RES(uint8_t bit, uint8_t value);
    
//interrupts
    void RST(uint8_t n);
    void interrupt(uint8_t n, uint8_t flag);
    int checkInterrupt();
public:
    CPU(MemoryMaster& master);
    void init();
    int step();

    void setInterrupt(InterruptState* master);
};
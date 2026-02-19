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


    uint16_t AF();
    uint16_t BC();
    uint16_t DE();
    uint16_t HL();

    void SETAF(uint16_t n);
    void SETBC(uint16_t n);
    void SETDE(uint16_t n);
    void SETHL(uint16_t n);

    uint16_t nn();
    uint8_t n();
    uint8_t& getReg(uint8_t registr, int& time);
// Exec opcodes
    int execute(uint8_t opcode);
    int executeCB(uint8_t opcode);
// math ops
    void SAVESP();
    void PUSH(uint16_t n);
    uint8_t POP();
    uint16_t POP16();
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
    uint8_t JNZ();
    uint8_t JZ();
    uint8_t JNC();
    uint8_t JC();
    uint8_t JRNZ();
    uint8_t JRZ();
    uint8_t JRNC();
    uint8_t JRC();
    void JR();
    uint8_t CALLNZ();
    uint8_t CALLZ();
    uint8_t CALLNC();
    uint8_t CALLC();
    void CALL();
    uint8_t RETNZ();
    uint8_t RETZ();
    uint8_t RETNC();
    uint8_t RETC();
    void RST(uint8_t n);
    void BIT(uint8_t bit, uint8_t value);
    void SET(uint8_t bit, uint8_t& value);
    void RES(uint8_t bit, uint8_t& value);
    void RLCA();
    void RRCA();
    void RRA();
    void RLA();
    void CPL();
    void CCF();
    void DAA();
    void SRL(uint8_t &reg);
    void SLA(uint8_t &reg);
    void SRA(uint8_t &reg);
    void RR(uint8_t &reg);
    void RL(uint8_t &reg);
    void RLC(uint8_t &reg);
    void RRC(uint8_t &reg);
    void SWAP(uint8_t& n);
    void STOP();
    
//interrupts
    void interrupt(uint8_t n, uint8_t flag);
public:
    void init();

    CPU(MemoryMaster& master);
    int checkInterrupt();
    int step();

    void setInterrupt(InterruptState* master);
};
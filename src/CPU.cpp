#include "../include/CPU.hpp"
#include "../include/MEM.hpp"

uint16_t CPU::AF(){ return uint16_t(A)<<8|F; }
uint16_t CPU::BC(){ return uint16_t(B)<<8|C; }
uint16_t CPU::DE(){ return uint16_t(D)<<8|E; }
uint16_t CPU::HL(){ return uint16_t(H)<<8|L; }

void CPU::SETAF(uint16_t nn){ A = (nn>>8); F = nn; }
void CPU::SETBC(uint16_t nn){ B = (nn>>8); C = nn; }
void CPU::SETDE(uint16_t nn){ D = (nn>>8); E = nn; }
void CPU::SETHL(uint16_t nn){ H = (nn>>8); L = nn; }

uint16_t CPU::nn() {
    uint8_t low = MEM.read(PC++);
    uint16_t high = MEM.read(PC++);
    return (high << 8) | low;
}
uint8_t CPU::n(){ return MEM.read(PC++); }
void CPU::SAVESP() {
    uint16_t adr = nn();
    MEM.write(adr, SP);
    MEM.write(adr + 1, SP >> 8);
}
void CPU::PUSH(uint16_t n) {
    MEM.write(--SP, n>>8);
    MEM.write(--SP, n);
}
uint8_t CPU::POP() {
    return MEM.read(SP++);
}
uint16_t CPU::POP16() {
    uint8_t low = MEM.read(SP++);
    uint8_t high = MEM.read(SP++);
    return (high << 8) | low;
}
void CPU::ADD8(uint8_t b) {
    uint16_t result = A + b;
    F = 0;
    
    if ((result & 0xFF) == 0) F |= zero;
    if (result > 0xFF) F |= car;
    if ((A & 0xF) + (b & 0xF) > 0xF) F |= hcar;
    A = result;
}
void CPU::ADC8(uint8_t b) {
    uint8_t carry = (F & car) ? 1 : 0;
    uint16_t result = A + b + carry;
    F = 0;
    if ((result & 0xFF) == 0) F |= zero;
    if (result > 0xFF) F |= car;
    if ((A & 0xF) + (b & 0xF) + carry > 0xF) F |= hcar;
    
    A = result;
}
void CPU::SUB8(uint8_t b) {
    int result = A - b;
    F = sub;
    
    if ((result & 0xFF) == 0) F |= zero;
    if (A < b) F |= car;
    if ((A & 0xF) < (b & 0xF)) F |= hcar;
    
    A = result;
}
void CPU::SBC8(uint8_t b) {
    uint8_t carry = (F & car) ? 1 : 0;
    int result = A - b - carry;
    F = sub;
    if ((result & 0xFF) == 0) F |= zero;
    if (A < b + carry) F |= car;
    if ((A & 0xF) < (b & 0xF) + carry) F |= hcar;
    
    A = result;
}
void CPU::AND8(uint8_t b) {
    A &= b;
    F = hcar;
    if (A == 0) F |= zero;
}
void CPU::OR8(uint8_t b) {
    A |= b;
    F = 0;
    if (A == 0) F |= zero;
}
void CPU::XOR8(uint8_t b) {
    A ^= b;
    F = 0;
    if (A == 0) F |= zero;
}
void CPU::CP8(uint8_t b) {
    uint8_t result = A - b;
    F = sub;
    
    if (result == 0) F |= zero;
    if (A < b) F |= car;
    if ((A & 0xF) < (b & 0xF)) F |= hcar;
}
uint8_t CPU::INC8(uint8_t b) {
    uint8_t result = b + 1;
    F &= car;
    if (result == 0) F |= zero;
    if ((b & 0xF) == 0xF) F |= hcar;
    return result;
}
uint8_t CPU::DEC8(uint8_t b) {
    uint8_t result = b - 1;
    F &= car;
    F |= sub;
    if (result == 0) F |= zero;
    if ((b & 0xF) == 0) F |= hcar;
    return result;
}
uint16_t CPU::ADD16(uint16_t a, uint16_t b) {
    uint32_t result = a + b;
    F &= zero;
    if ((a & 0x0FFF) + (b & 0x0FFF) > 0x0FFF) F |= hcar;
    if (result > 0xFFFF) F |= car;

    return result;
}
uint16_t CPU::ADD16S(uint16_t nn, int8_t e) {
    uint16_t result = nn + e;
    F = 0;
    if (((nn ^ e ^ result) & car) != 0) F |= hcar;
    if (((nn ^ e ^ result) & 0x100) != 0) F |= car;
    
    return result;
}

uint8_t CPU::JNZ() {
    if (!(F & zero)){
        PC = nn();
        return 16;
    }
    PC += 2;
    return 12;
}
uint8_t CPU::JZ() {
    if ((F & zero)){
        PC = nn();
        return 16;
    }
    PC += 2;
    return 12;
}
uint8_t CPU::JNC() {
    if (!(F & car)){
        PC = nn();
        return 16;
    }
    PC += 2;
    return 12;
}
uint8_t CPU::JC() {
    if ((F & car)){
        PC = nn();
        return 16;
    }
    PC += 2;
    return 12;
}
uint8_t CPU::JRNZ() {
    if (!(F & zero)){
        JR();
        return 12;
    }
    PC ++;
    return 8;
}
uint8_t CPU::JRZ() {
    if ((F & zero)){
        JR();
        return 12;
    }
    PC ++;
    return 8;
}
uint8_t CPU::JRNC() {
    if (!(F & car)){
        JR();
        return 12;
    }
    PC ++;
    return 8;
}
uint8_t CPU::JRC() {
    if ((F & car)){
        JR();
        return 12;
    }
    PC ++;
    return 8;
}
void CPU::JR() {
    PC += int8_t(n());
}
uint8_t CPU::CALLNZ() {
    if (!(F & zero)){
        CALL();
        return 24;
    }
    PC+=2;
    return 12;
}
uint8_t CPU::CALLZ() {
    if ((F & zero)){
        CALL();
        return 24;
    }
    PC+=2;
    return 12;
}
uint8_t CPU::CALLNC() {
    if (!(F & car)){
        CALL();
        return 24;
    }
    PC+=2;
    return 12;
}
uint8_t CPU::CALLC() {
    if ((F & car)){
        CALL();
        return 24;
    }
    PC+=2;
    return 12;
}
void CPU::CALL() {
    uint16_t adr = nn();
    PUSH(PC);
    PC = adr;
}
uint8_t CPU::RETNZ() {
    if (!(F & zero)){
        PC = POP16();
        return 20;
    }
    return 8;
}
uint8_t CPU::RETZ() {
    if ((F & zero)){
        PC = POP16();
        return 20;
    }
    return 8;
}
uint8_t CPU::RETNC() {
    if (!(F & car)){
        PC = POP16();
        return 20;
    }
    return 8;
}
uint8_t CPU::RETC() {
    if ((F & car)){
        PC = POP16();
        return 20;
    }
    return 8;
}
void CPU::RST(uint8_t n) {
    PUSH(PC);
    PC = uint16_t(n);
}

void CPU::BIT(uint8_t bit, uint8_t value) {
    bool bit_value = (value >> bit) & 1;
    uint8_t old_carry = F & car;
    
    F = (!bit_value) ? zero : 0;
    F |= hcar;
    F |= old_carry;
}
void CPU::SET(uint8_t bit, uint8_t& value) {
    value |= (1 << bit);
}
void CPU::RES(uint8_t bit, uint8_t& value) {
    value &= ~(1 << bit);
}

void CPU::RLCA() {
    bool old_bit7 = (A >> 7) & 1;
    A = (A << 1) | old_bit7;
    F = 0;
    if (old_bit7) F |= car;
}
void CPU::RRCA() {
    bool old_bit0 = A & 1;
    A = (A >> 1) | (old_bit0 << 7);
    F = 0;
    if (old_bit0) F |= car;
}
void CPU::RRA() {
    bool bit0 = A & 1;
    A = (A >> 1) | ((F & car) ? zero : 0);
    F = (bit0 ? car : 0);
}
void CPU::RLA() {
    bool old_bit7 = (A >> 7) & 1;
    A = (A << 1) | ((F & car) ? 1 : 0);
    F = 0;
    if (old_bit7) F |= car;
}
void CPU::CPL() {
    A = ~A;
    F |= sub | hcar;
}
void CPU::CCF() {
    bool z_flag = (F & zero) != 0;
    bool old_carry = (F & car) != 0;
    
    F = 0;
    if (z_flag) F |= zero;
    if (!old_carry) F |= car;
}
void CPU::DAA() {
    uint8_t a = A;
    uint8_t adjust = 0;
    uint8_t fc = (F & car);
    uint8_t fh = (F & hcar);
    uint8_t fn = (F & sub);
    
    if (!fn) {
        if (fh || (a & 0x0F) > 0x09) {
            adjust += 0x06;
        }
        if (fc || a > 0x99) {
            adjust += 0x60;
            fc = car;
        }
    } 
    else {
        if (fh) {
            adjust -= 0x06;
        }
        if (fc) {
            adjust -= 0x60;
            fc = car;
        }
    }
    A = a + adjust;
    F = fc;
    if (A == 0) F |= zero;
    if (fn) F |= sub;
}
void CPU::SRL(uint8_t &reg) {
    bool bit0 = reg & 1;
    reg = reg >> 1;
    F = 0;
    if (reg == 0) F |= zero;
    if (bit0) F |= car;
}
void CPU::SLA(uint8_t &reg) {
    uint8_t bit7 = (reg >> 7) & 1;
    reg = reg << 1;
    
    F = 0;
    if (reg == 0) F |= zero;
    if (bit7) F |= car;
}
void CPU::SRA(uint8_t &reg) {
    uint8_t bit0 = reg & 1;
    uint8_t bit7 = reg & zero;
    
    reg = (reg >> 1) | bit7;
    F = 0;
    if (reg == 0) F |= zero;
    if (bit0) F |= car;
}
void CPU::RR(uint8_t &reg) {
    uint8_t old_carry = (F >> 4) & 1;
    uint8_t bit0 = reg & 1;
    reg = (reg >> 1) | (old_carry << 7);
    F = 0;
    
    if (reg == 0) F |= zero;
    if (bit0) F |= car;
}
void CPU::RL(uint8_t &reg) {
    uint8_t old_carry = (F >> 4) & 1;
    uint8_t bit7 = (reg >> 7) & 1;
    reg = (reg << 1) | old_carry;
    F = 0;
    if (reg == 0) F |= zero;
    if (bit7) F |= car;
}
void CPU::RLC(uint8_t &reg) {
    uint8_t bit7 = (reg >> 7) & 1;
    reg = (reg << 1) | bit7;
    F = 0;
    if (reg == 0) F |= zero;
    if (bit7) F |= car;
}
void CPU::RRC(uint8_t &reg) {
    uint8_t bit0 = reg & 1;
    reg = (reg >> 1) | (bit0 << 7);
    F = 0;
    if (reg == 0) F |= zero;
    if (bit0) F |= car;
}
void CPU::SWAP(uint8_t& n) {
    n = (n << 4) | (n >> 4);
    F = 0;
    if (n == 0) F |= zero;
}
void CPU::STOP(){
    if (MEM.isCGB && (MEM.readIO(0xFF4D) & 1)){
        MEM.doubleCPUspeed = !MEM.doubleCPUspeed;
    }
    else if (ime) halt = true;
}
CPU::CPU(MemoryMaster& master) : MEM(master){}
void CPU::init(){
    MEM.write(0xFF40, 0x91);  // LCDC

    MEM.write(0xFF47, 0xFC);  // BGP
    MEM.write(0xFF48, 0xFF);  // OBP0
    MEM.write(0xFF49, 0xFF);  // OBP1
    if (MEM.isCGB){
        MEM.write(0xFF70, 1); // SVBK = 1
    }
    //
    /*MEM[0xFF05] = 0x00;  // TIMA
    MEM[0xFF06] = 0x00;  // TMA
    MEM[0xFF07] = 0x00;  // TAC
    MEM[0xFF10] = 0x80;  // NR10
    MEM[0xFF11] = 0xBF;  // NR11
    MEM[0xFF12] = 0xF3;  // NR12
    MEM[0xFF14] = 0xBF;  // NR14
    MEM[0xFF16] = 0x3F;  // NR21
    MEM[0xFF17] = 0x00;  // NR22
    MEM[0xFF19] = 0xBF;  // NR24
    MEM[0xFF1A] = 0x7F;  // NR30
    MEM[0xFF1B] = 0xFF;  // NR31
    MEM[0xFF1C] = 0x9F;  // NR32
    MEM[0xFF1E] = 0xBF;  // NR33
    MEM[0xFF20] = 0xFF;  // NR41
    MEM[0xFF21] = 0x00;  // NR42
    MEM[0xFF22] = 0x00;  // NR43
    MEM[0xFF23] = 0xBF;  // NR30
    MEM[0xFF24] = 0x77;  // NR50
    MEM[0xFF25] = 0xF3;  // NR51
    MEM[0xFF26] = 0xF1;  // NR52 (0xF1 for DMG, 0xF0 for SGB)
    MEM[0xFF40] = 0x91;  // LCDC
    MEM[0xFF42] = 0x00;  // SCY
    MEM[0xFF43] = 0x00;  // SCX
    MEM[0xFF45] = 0x00;  // LYC
    MEM[0xFF47] = 0xFC;  // BGP
    MEM[0xFF48] = 0xFF;  // OBP0
    MEM[0xFF49] = 0xFF;  // OBP1
    MEM[0xFF4A] = 0x00;  // WY
    MEM[0xFF4B] = 0x00;  // WX
    MEM[0xFFFF] = 0x00;  // IE*/
}
uint8_t& CPU::getReg(uint8_t registr, int& time){
    switch (registr) {
        case 0x00: return B;
        case 0x01: return C;
        case 0x02: return D;
        case 0x03: return E;
        case 0x04: return H;
        case 0x05: return L;
        case 0x06: time *= 2; return MEM.read(HL());
        case 0x07: return A;
    }
    return F;
}
int CPU::execute(uint8_t opcode){
    uint8_t pcode = opcode >> 6;
    uint8_t code = opcode >> 3 & 0x7;
    uint8_t registr = opcode & 0x7;
    int time = 4;
    switch (pcode) {
        case 0:
            switch (opcode) {
                case 0x00: return 4;
                case 0x01: SETBC(nn()); return 12;
                case 0x02: MEM.write(BC(), A); return 8;
                case 0x03: SETBC(BC()+1); return 8;
                case 0x04: B = INC8(B); return 4;
                case 0x05: B = DEC8(B); return 4;
                case 0x06: B = n(); return 8;
                case 0x07: RLCA(); return 4;
                case 0x08: SAVESP(); return 20;
                case 0x09: SETHL(ADD16(HL(),BC())); return 8;
                case 0x0A: A = MEM.read(BC()); return 8;
                case 0x0B: SETBC(BC()-1); return 8;
                case 0x0C: C = INC8(C); return 4;
                case 0x0D: C = DEC8(C); return 4;
                case 0x0E: C = n(); return 8;
                case 0x0F: RRCA(); return 4;
                case 0x10: STOP(); return 4;
                case 0x11: SETDE(nn()); return 12;
                case 0x12: MEM.write(DE(), A); return 8;
                case 0x13: SETDE(DE()+1);; return 8;
                case 0x14: D = INC8(D); return 4;
                case 0x15: D = DEC8(D); return 4;
                case 0x16: D = n(); return 8;
                case 0x17: RLA(); return 4;
                case 0x18: JR(); return 12;
                case 0x19: SETHL(ADD16(HL(),DE())); return 8;
                case 0x1A: A = MEM.read(DE()); return 8;
                case 0x1B: SETDE(DE()-1); return 8;
                case 0x1C: E = INC8(E); return 4;
                case 0x1D: E = DEC8(E); return 4;
                case 0x1E: E = n(); return 8;
                case 0x1F: RRA(); return 4;
                case 0x20: return JRNZ();
                case 0x21: SETHL(nn()); return 12;
                case 0x22: MEM.write(HL(), A); SETHL(HL()+1); return 8;
                case 0x23: SETHL(HL()+1); return 8;
                case 0x24: H = INC8(H); return 4;
                case 0x25: H = DEC8(H); return 4;
                case 0x26: H = n(); return 8;
                case 0x27: DAA(); return 4;
                case 0x28: return JRZ();
                case 0x29: SETHL(ADD16(HL(),HL())); return 8;
                case 0x2A: A = MEM.read(HL()); SETHL(HL()+1); return 8;
                case 0x2B: SETHL(HL()-1); return 8;
                case 0x2C: L = INC8(L); return 4;
                case 0x2D: L = DEC8(L); return 4;
                case 0x2E: L = n(); return 8;
                case 0x2F: CPL(); return 4;
                case 0x30: return JRNC();
                case 0x31: SP = nn(); return 12;
                case 0x32: MEM.write(HL(), A); SETHL(HL()-1); return 8;
                case 0x33: SP++; return 8;
                case 0x34: MEM.write(HL(), INC8(MEM.read(HL()))); return 12;
                case 0x35: MEM.write(HL(), DEC8(MEM.read(HL()))); return 12;
                case 0x36: MEM.write(HL(), n()); return 12;
                case 0x37: F &= zero; F |= car; return 4;
                case 0x38: return JRC();
                case 0x39: SETHL(ADD16(HL(),SP)); return 8;
                case 0x3A: A = MEM.read(HL()); SETHL(HL()-1); return 8;
                case 0x3B: SP--; return 8;
                case 0x3C: A = INC8(A); return 4;
                case 0x3D: A = DEC8(A); return 4;
                case 0x3E: A = n(); return 8;
                case 0x3F: CCF(); return 4;
            }
            break;
        case 1:{
            if (opcode == 0x76){
                halt = true;
                break;
            }
            uint8_t& out = getReg(code, time);
            uint8_t& inp = getReg(registr, time);
            out = inp;
        } break;
        case 2:{
            uint8_t out = getReg(registr, time);
            switch (code) {
                case 0x00: ADD8(out); break;
                case 0x01: ADC8(out); break;
                case 0x02: SUB8(out); break;
                case 0x03: SBC8(out); break;
                case 0x04: AND8(out); break;
                case 0x05: XOR8(out); break;
                case 0x06: OR8(out); break;
                case 0x07: CP8(out); break;
            }
        }break;
        case 3:{
            switch (opcode) {
                case 0xC0: return RETNZ();
                case 0xC1: C = POP(); B = POP(); return 12;
                case 0xC2: return JNZ();
                case 0xC3: PC = nn(); return 16;
                case 0xC4: return CALLNZ();
                case 0xC5: PUSH(BC()); return 16;
                case 0xC6: ADD8(n()); return 8;
                case 0xC7: RST(0x00); return 16;
                case 0xC8: return RETZ();
                case 0xC9: PC = POP16(); return 16;
                case 0xCA: return JZ();
                case 0xCB: return executeCB(n());

                case 0xD0: return RETNC();
                case 0xD8: return RETC();

                case 0xD2: return JNC();
                case 0xDA: return JC();
                case 0xE9: PC = HL(); return 4;
                
                case 0xCC: return CALLZ();
                case 0xCD: CALL(); return 24;
                case 0xD4: return CALLNC();
                case 0xDC: return CALLC();

                case 0xD5: PUSH(DE()); return 16;
                case 0xE5: PUSH(HL()); return 16;
                case 0xF5: PUSH(AF()); return 16;

                case 0xD1: E = POP(); D = POP(); return 12;
                case 0xE1: L = POP(); H = POP(); return 12;
                case 0xF1: F = POP() & 0xF0; A = POP(); return 12;

                case 0xCE: ADC8(n()); return 8;
                case 0xD6: SUB8(n()); return 8;
                case 0xDE: SBC8(n()); return 8;

                case 0xD9: PC = POP16(); ime = true; return 16;

                case 0xD7: RST(0x10); return 16;
                case 0xE7: RST(0x20); return 16;
                case 0xF7: RST(0x30); return 16;
                case 0xCF: RST(0x08); return 16;
                case 0xDF: RST(0x18); return 16;
                case 0xEF: RST(0x28); return 16;
                case 0xFF: RST(0x38); return 16;

                case 0xE0: MEM.write(0xFF00 + n(), A); return 12;
                case 0xE2: MEM.write(0xFF00 + C, A); return 8;
                case 0xE6: AND8(n()); return 8;

                case 0xE8: SP = ADD16S(SP, n()); return 16;
                case 0xF8: SETHL(ADD16S(SP, n())); return 12;

                case 0xEA: MEM.write(nn(), A); return 16;
                case 0xEE: XOR8(n()); return 8;
                case 0xF0: A = MEM.read(0xFF00 + n()); return 12;
                case 0xF2: A = MEM.read(0xFF00 + C); return 8;
                case 0xF3: ime = false; return 4;
                case 0xF6: OR8(n()); return 8;
                case 0xF9: SP = HL(); return 8;
                case 0xFA: A = MEM.read(nn()); return 16;
                case 0xFB: ime = true; return 4;
                case 0xFE: CP8(n()); return 8;
            }
        }break;
    }
    return time;
}
int CPU::executeCB(uint8_t opcode){
    uint8_t pcode = opcode >> 6;
    uint8_t code = opcode >> 3 & 0x7;
    uint8_t registr = opcode & 0x7;
    int time = 8;
    uint8_t& out = getReg(registr, time);
    switch (pcode) {
        case 0:
            switch (code) {
                case 0x00: RLC(out); break;
                case 0x01: RRC(out); break;
                case 0x02: RL(out); break;
                case 0x03: RR(out); break;
                case 0x04: SLA(out); break;
                case 0x05: SRA(out); break;
                case 0x06: SWAP(out); break;
                case 0x07: SRL(out); break;
            }
            break;
        case 1: // BIT [HL] - 12
            if (time == 16) time = 12;
            BIT(code,out);
            break;
        case 2:
            RES(code,out);
            break;
        case 3:
            SET(code,out);
            break;
    }
    return time;
}
void CPU::interrupt(uint8_t n, uint8_t flag){
    IS->IF &= (~flag);
    RST(n);
    ime = false;
}

int CPU::checkInterrupt(){
    uint8_t pending = IS->IE & IS->IF;
    
    if (pending == 0) return 0;
    if (halt) halt = false;
    if (!ime) return 0;

    if (pending & VBLANK) {
        interrupt(0x40, 0x01);
        return 20;
    }
    if (pending & STAT) {
        interrupt(0x48, 0x02);
        return 20;
    }
    if (pending & TIMER) {
        interrupt(0x50, 0x04);
        return 20;
    }
    /*if (pending & SERIAL) {
        interrupt(0x58, 0x08);
        return 20;
    }*/
    if (pending & INPUT) {
        interrupt(0x60, 0x10);
        return 20;
    }
    return 0;
}
int CPU::step(){
    int time = checkInterrupt();
    if(!halt) {
        uint8_t code = n();
        time += execute(code);
        // timers works on standart speed
        if (MEM.doubleCPUspeed) execute(n());
    }else time += 4;
    return time;
}

void CPU::setInterrupt(InterruptState* master){
    IS = master;
}
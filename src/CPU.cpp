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

void CPU::CALL() {
    uint16_t adr = nn();
    PUSH(PC);
    PC = adr;
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
uint8_t CPU::SET(uint8_t bit, uint8_t value) {
    return value | (1 << bit);
}
uint8_t CPU::RES(uint8_t bit, uint8_t value) {
    return value & ~(1 << bit);
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
void CPU::SCF() {
    F &= zero;
    F |= car;
}
void CPU::CCF() {
    bool z_flag = (F & zero);
    bool old_carry = (F & car);
    
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
uint8_t CPU::SRL(uint8_t reg) {
    bool bit0 = reg & 1;
    uint8_t out = reg >> 1;
    F = 0;
    if (out == 0) F |= zero;
    if (bit0) F |= car;
    return out;
}
uint8_t CPU::SLA(uint8_t reg) {
    uint8_t bit7 = (reg >> 7) & 1;
    uint8_t out = reg << 1;
    
    F = 0;
    if (out == 0) F |= zero;
    if (bit7) F |= car;
    return out;
}
uint8_t CPU::SRA(uint8_t reg) {
    uint8_t bit0 = reg & 1;
    uint8_t bit7 = reg & zero;
    
    uint8_t out = (reg >> 1) | bit7;
    F = 0;
    if (out == 0) F |= zero;
    if (bit0) F |= car;
    return out;
}
uint8_t CPU::RR(uint8_t reg) {
    uint8_t old_carry = (F >> 4) & 1;
    uint8_t bit0 = reg & 1;
    uint8_t out = (reg >> 1) | (old_carry << 7);
    F = 0;
    
    if (out == 0) F |= zero;
    if (bit0) F |= car;
    return out;
}

uint8_t CPU::RL(uint8_t reg) {
    uint8_t old_carry = (F >> 4) & 1;
    uint8_t bit7 = (reg >> 7) & 1;
    uint8_t out = (reg << 1) | old_carry;
    F = 0;
    if (out == 0) F |= zero;
    if (bit7) F |= car;
    return out;
}
uint8_t CPU::RLC(uint8_t reg) {
    uint8_t bit7 = (reg >> 7) & 1;
    uint8_t out = (reg << 1) | bit7;
    F = 0;
    if (out == 0) F |= zero;
    if (bit7) F |= car;
    return out;
}
uint8_t CPU::RRC(uint8_t reg) {
    uint8_t bit0 = reg & 1;
    uint8_t out = (reg >> 1) | (bit0 << 7);
    F = 0;
    if (out == 0) F |= zero;
    if (bit0) F |= car;
    return out;
}
uint8_t CPU::SWAP(uint8_t reg) {
    uint8_t out = (reg << 4) | (reg >> 4);
    F = 0;
    if (out == 0) F |= zero;
    return out;
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
}
uint8_t CPU::readTableR(uint8_t code){
    switch (code) {
        case (0): return B;
        case (1): return C;
        case (2): return D;
        case (3): return E;
        case (4): return H;
        case (5): return L;
        case (6):
            extraTime += 4;
            return MEM.read(HL());
        case (7): return A;
    }
    return 0xFF;
}
uint16_t CPU::readTableRP(uint8_t code){
    switch (code) {
        case (0): return BC();
        case (1): return DE();
        case (2): return HL();
        case (3): return SP;
    }
    return 0xFFFF;
}
uint16_t CPU::readTableRP2(uint8_t code){
    switch (code) {
        case (0): return BC();
        case (1): return DE();
        case (2): return HL();
        case (3): return AF();
    }
    return 0xFFFF;
}
void CPU::writeTableR(uint8_t code, uint8_t data){
    switch (code) {
        case (0): B = data; break;
        case (1): C = data; break;
        case (2): D = data; break;
        case (3): E = data; break;
        case (4): H = data; break;
        case (5): L = data; break;
        case (6):
            extraTime += 4;
            MEM.write(HL(), data);
            break;
        case (7): A = data; break;
    }
}
void CPU::writeTableRP(uint8_t code, uint16_t data){
    switch (code) {
        case (0): SETBC(data); break;
        case (1): SETDE(data); break;
        case (2): SETHL(data); break;
        case (3): SP = data; break;
    }
}
void CPU::writeTableRP2(uint8_t code, uint16_t data){
    switch (code) {
        case (0): SETBC(data); break;
        case (1): SETDE(data); break;
        case (2): SETHL(data); break;
        case (3): SETAF(data); break;
    }
}
void CPU::executeTableALU(uint8_t code, uint8_t data){
    switch (code) {
        case 0x00: ADD8(data); break;
        case 0x01: ADC8(data); break;
        case 0x02: SUB8(data); break;
        case 0x03: SBC8(data); break;
        case 0x04: AND8(data); break;
        case 0x05: XOR8(data); break;
        case 0x06: OR8(data); break;
        case 0x07: CP8(data); break;
    }
}
bool CPU::readTableCC(uint8_t code){
    switch (code) {
        case(0): return !(F & zero);
        case(1): return (F & zero);
        case(2): return !(F & car);
        case(3): return (F & car);
    }
    return false;
}
void CPU::executeTableACC(uint8_t code){
    switch (code) {
        case 0x00: RLCA(); break;
        case 0x01: RRCA(); break;
        case 0x02: RLA(); break;
        case 0x03: RRA(); break;
        case 0x04: DAA(); break;
        case 0x05: CPL(); break;
        case 0x06: SCF(); break;
        case 0x07: CCF(); break;
    }
}
void CPU::executeTableRJAO(uint8_t code){
    switch (code) {
        case 0x00: break;
        case 0x01:
            SAVESP();
            extraTime += 16;
            break;
        case 0x02: STOP(); break;
        case 0x03:
            PC += int8_t(n());
            extraTime += 8;
            break;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            if (readTableCC(code - 4)){
                PC += int8_t(n());
                extraTime += 8;
            } else{
                PC += 1;
                extraTime += 4;
            } break;
    }
}
void CPU::executeTableCRAO(uint8_t code){
    switch (code) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            if (readTableCC(code)){
                PC = POP16();
                extraTime += 16;
            }else {
                extraTime += 4;
            } break;
        case 0x04:
            MEM.write(0xFF00 + n(), A);
            extraTime += 8;
            break;
        case 0x05:
            extraTime += 12;
            SP = ADD16S(SP, n());
            break;
        case 0x06:
            extraTime += 8;
            A = MEM.read(0xFF00 + n());
            break;
        case 0x07:
            extraTime += 8;
            SETHL(ADD16S(SP, n()));
            break;
    }
}
void CPU::executeTableCJAO(uint8_t code){
    switch (code) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            if (readTableCC(code)){
                PC = nn();
                extraTime += 8;
            }else {
                PC += 2;
                extraTime += 4;
            } break;
        case 0x04: MEM.write(0xFF00+C, A); break;
        case 0x05:
            MEM.write(nn(), A);
            extraTime += 8;
            break;
        case 0x06: A = MEM.read(0xFF00+C); break;
        case 0x07:
            A = MEM.read(nn());
            extraTime += 8;
            break;
    }
}
uint8_t CPU::executeTableROT(uint8_t code, uint8_t data){
    switch (code) {
        case 0x00: return RLC(data);
        case 0x01: return RRC(data);
        case 0x02: return RL(data);
        case 0x03: return RR(data);
        case 0x04: return SLA(data);
        case 0x05: return SRA(data);
        case 0x06: return SWAP(data);
        case 0x07: return SRL(data);
    }
    return 0xFF;
}
int CPU::execute(uint8_t opcode){
    extraTime = 0;
    uint8_t x = opcode >> 6;
    uint8_t y = opcode >> 3 & 0x7;
    uint8_t z = opcode & 0x7;

    uint8_t q = y & 1;
    uint8_t p = (y >> 1);
    switch (x) {
        case 0:
            switch (z) {
                case (0):
                    executeTableRJAO(y);
                    return 4;
                case (1):{
                    if (q) {
                        SETHL(ADD16(HL(), readTableRP(p)));
                        return 8;
                    }
                    writeTableRP(p, nn());
                } return 12;
                case (2):{
                    uint16_t dst;
                    switch (p) {
                        case (0): dst = BC(); break;
                        case (1): dst = DE(); break;
                        case (2): dst = HL(); SETHL(HL()+1); break;
                        case (3): dst = HL(); SETHL(HL()-1); break; 
                    }
                    if (q) A = MEM.read(dst);
                    else MEM.write(dst, A);
                } return 8;
                case (3):{
                    if (q) writeTableRP(p, readTableRP(p) - 1);
                    else writeTableRP(p, readTableRP(p) + 1);
                } return 8;
                case (4):
                    writeTableR(y, INC8(readTableR(y)));
                    return 4;
                case (5):
                    writeTableR(y, DEC8(readTableR(y)));
                    return 4;
                case (6):
                    writeTableR(y, n());
                    return 8;
                case (7):
                    executeTableACC(y);
                    return 4;
            }
            break;
        case 1:
            if (opcode == 0x76){
                halt = true;
                break;
            }
            writeTableR(y, readTableR(z));
            return 4;
        case 2:
            executeTableALU(y, readTableR(z));
            return 4;
        case 3:{
            switch (z) {
                case (0):
                    executeTableCRAO(y);
                    return 4;
                case (1):{
                    if (q){
                        switch (p) {
                            case (0): PC = POP16(); return 16;
                            case (1): PC = POP16(); ime = true; return 16;
                            case (2): PC = HL(); return 4;
                            case (3): SP = HL(); return 8;
                        }
                    }
                    else writeTableRP2(p, POP16());
                    } return 12;
                case (2):
                    executeTableCJAO(y);
                    return 8;
                case (3):
                    switch (y) {
                        case (0): PC = nn(); return 16;
                        case (1): return executeCB(n());
                        case (6): ime = false; break;
                        case (7): ime = true; break;
                    }
                    return 4;
                case (4):
                    if (readTableCC(y)){
                        CALL();
                        extraTime += 12;
                    } else{
                        PC += 2;
                    }return 12;
                case (5):{
                    if (q){
                        CALL();
                        extraTime += 8;
                    }
                    else PUSH(readTableRP2(p));
                    } return 16;
                case (6):
                    executeTableALU(y, n());
                    return 8;
                case (7):
                    RST(y*8);
                    return 16;
            }
        }break;
    }
    return 0;
}
int CPU::executeCB(uint8_t opcode){
    uint8_t x = opcode >> 6;
    uint8_t y = opcode >> 3 & 0x7;
    uint8_t z = opcode & 0x7;
    uint8_t out = readTableR(z);
    switch (x) {
        case 0:
            writeTableR(z, executeTableROT(y , out));
            break;
        case 1:
            BIT(y,out);
            break;
        case 2:
            writeTableR(z, RES(y,out));
            break;
        case 3:
            writeTableR(z, SET(y,out));
            break;
    }
    return 8;
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
    return time + extraTime;
}

void CPU::setInterrupt(InterruptState* master){
    IS = master;
}

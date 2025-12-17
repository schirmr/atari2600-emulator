#include "mos6502r.hpp" // Processador MOS 6502 Reduzido ou 6507
#include <iostream>

Mos6502::Mos6502(Memory* mem){ // Construtor
    this->memory = mem;
}

uint8_t Mos6502::busca(){ // Busca o próximo opcode
    return memory->read(PC++);
}

void Mos6502::LDA(uint16_t address){ // Load Accumulator
    A = memory->read(address);
    updateZN(A);
}

void Mos6502::updateZN(uint8_t value){ // Atualiza flags Zero e Negative
    setFlag(ZERO, value == 0x00);
    setFlag(NEGATIVE, (value & 0x80) != 0);
}

void Mos6502::IRQ() { // Interrupt Request
    if (getFlag(INTERRUPT_DISABLE) == 1) return;

    push((PC >> 8) & 0xFF);
    push(PC & 0xFF);

    push(status | UNUSED); 

    // 3. Desabilita futuras interrupções
    setFlag(INTERRUPT_DISABLE, true);

    // 4. Lê o vetor de interrupção e pula para lá
    uint16_t vector_addr = 0xFFFE;
    PC = read_word(vector_addr); 
    cycles += 7; // Interrupções custam 7 ciclos
}

void Mos6502::NMI() { // Non-Maskable Interrupt
    push((PC >> 8) & 0xFF);
    push(PC & 0xFF);
    push(status | UNUSED);
    
    setFlag(INTERRUPT_DISABLE, true);
    
    PC = read_word(0xFFFA); 
    cycles += 7;
}

void Mos6502::ADC(uint16_t address){ // Add with Carry
    uint8_t m = memory->read(address);
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;

    uint16_t bin_sum = (uint16_t)A + (uint16_t)m + (uint16_t)carry_in;
    
    bool overflow = (~(A ^ m) & (A ^ bin_sum) & 0x80) != 0;
    setFlag(OVERFLOW, overflow);

    updateZN((uint8_t)(bin_sum & 0xFF));

    if(getFlag(DECIMAL_MODE)){ 
        uint16_t low = (A & 0x0F) + (m & 0x0F) + carry_in;
        uint16_t high = (A & 0xF0) + (m & 0xF0);

        if (low > 9) {
            low += 6;
        }
        
        if (low > 0x0F) {
            high += 0x10;
        }
        
        low &= 0x0F; 

        bool decimal_carry = (high > 0x90); 
        if (decimal_carry) {
            high += 0x60;
        }
        
        A = (low | (high & 0xF0)) & 0xFF;
        
        setFlag(CARRY, decimal_carry);
    } else {
        // Modo Binário Normal 
        setFlag(CARRY, bin_sum > 0xFF);
        A = (uint8_t)(bin_sum & 0xFF);
    }
}

void Mos6502::SBC(uint16_t address){
    uint8_t m = memory->read(address);
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;
    
    uint16_t bin_diff = (uint16_t)A - (uint16_t)m - (uint16_t)(1 - carry_in);
    
    bool overflow = ((A ^ bin_diff) & (A ^ m) & 0x80) != 0;
    setFlag(OVERFLOW, overflow);

    setFlag(CARRY, !(bin_diff & 0xFF00)); 

    updateZN((uint8_t)(bin_diff & 0xFF));

    if(getFlag(DECIMAL_MODE)){
        
        uint16_t low = (A & 0x0F) - (m & 0x0F) - (1 - carry_in);
        uint16_t high = (A >> 4) - (m >> 4);

        if (low & 0x10) {
            low -= 6; 
            high--;   
        }
        
        if (high & 0x10) {
            high -= 6; 
        }

        A = (low & 0x0F) | ((high << 4) & 0xF0);        
    } else {
        A = (uint8_t)(bin_diff & 0xFF);
    }
}

void Mos6502::STA(uint16_t address){
    memory->write(address, A);
}

void Mos6502::LDX(uint16_t address){
    X = memory->read(address);
    updateZN(X);
}

void Mos6502::LDY(uint16_t address){
    Y = memory->read(address);
    updateZN(Y);
}

void Mos6502::AND(uint16_t address){
    uint8_t m = memory->read(address);
    A = A & m;
    updateZN(A); // affects Z and N
}

void Mos6502::CMP(uint16_t address){
    uint8_t m = memory->read(address);
    uint16_t res = (uint16_t)A - (uint16_t)m;
    setFlag(CARRY, A >= m); // set if A >= M
    setFlag(ZERO, (uint8_t)res == 0);
    setFlag(NEGATIVE, (res & 0x80) != 0);
}

void Mos6502::CPX(uint16_t address){
    uint8_t m = memory->read(address);
    uint16_t res = (uint16_t)X - (uint16_t)m;
    setFlag(CARRY, X >= m); // set if X >= M
    setFlag(ZERO, (uint8_t)res == 0);
    setFlag(NEGATIVE, (res & 0x80) != 0);
}

void Mos6502::CPY(uint16_t address){
    uint8_t m = memory->read(address);
    uint16_t res = (uint16_t)Y - (uint16_t)m;
    setFlag(CARRY, Y >= m); // set if Y >= M
    setFlag(ZERO, (uint8_t)res == 0);
    setFlag(NEGATIVE, (res & 0x80) != 0);
}

void Mos6502::ASL_A(){
    // Carry gets original bit 7
    setFlag(CARRY, (A & 0x80) != 0);
    A = (uint8_t)((A << 1) & 0xFF);
    updateZN(A);
}

void Mos6502::ASL(uint16_t address){
    uint8_t v = memory->read(address);
    setFlag(CARRY, (v & 0x80) != 0);
    uint8_t res = (uint8_t)((v << 1) & 0xFF);
    memory->write(address, res);
    updateZN(res);
}

void Mos6502::LSR_A(){
    setFlag(CARRY, (A & 0x01) != 0);
    A = (uint8_t)(A >> 1);
    updateZN(A);
}

void Mos6502::LSR(uint16_t address){
    uint8_t v = memory->read(address);
    setFlag(CARRY, (v & 0x01) != 0);
    uint8_t res = (uint8_t)(v >> 1);
    memory->write(address, res);
    updateZN(res);
}

void Mos6502::ROL_A(){
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;
    bool new_carry = (A & 0x80) != 0;
    A = (uint8_t)(((A << 1) & 0xFF) | carry_in);
    setFlag(CARRY, new_carry);
    updateZN(A);
}

void Mos6502::ROL(uint16_t address){
    uint8_t v = memory->read(address);
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;
    bool new_carry = (v & 0x80) != 0;
    uint8_t res = (uint8_t)(((v << 1) & 0xFF) | carry_in);
    memory->write(address, res);
    setFlag(CARRY, new_carry);
    updateZN(res);
}

void Mos6502::ROR_A(){
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;
    bool new_carry = (A & 0x01) != 0;
    A = (uint8_t)((A >> 1) | (carry_in << 7));
    setFlag(CARRY, new_carry);
    updateZN(A);
}

void Mos6502::ROR(uint16_t address){
    uint8_t v = memory->read(address);
    uint8_t carry_in = getFlag(CARRY) ? 1 : 0;
    bool new_carry = (v & 0x01) != 0;
    uint8_t res = (uint8_t)((v >> 1) | (carry_in << 7));
    memory->write(address, res);
    setFlag(CARRY, new_carry);
    updateZN(res);
}

void Mos6502::BIT(uint16_t address){
    uint8_t m = memory->read(address);
    // Z flag: (A & M) == 0
    setFlag(ZERO, (uint8_t)(A & m) == 0);
    // N and V reflect bits 7 and 6 of memory
    setFlag(NEGATIVE, (m & 0x80) != 0);
    setFlag(OVERFLOW, (m & 0x40) != 0);
}

void Mos6502::DEC(uint16_t address){
    uint8_t v = memory->read(address);
    v = (uint8_t)(v - 1);
    memory->write(address, v);
    updateZN(v);
}

void Mos6502::INC(uint16_t address){
    uint8_t v = memory->read(address);
    v = (uint8_t)(v + 1);
    memory->write(address, v);
    updateZN(v);
}

void Mos6502::EOR(uint16_t address){
    uint8_t m = memory->read(address);
    A = (uint8_t)(A ^ m);
    updateZN(A);
}

void Mos6502::ORA(uint16_t address){
    uint8_t m = memory->read(address);
    A = (uint8_t)(A | m);
    updateZN(A);
}

void Mos6502::STX(uint16_t address){
    memory->write(address, X);
}

void Mos6502::STY(uint16_t address){
    memory->write(address, Y);
}

void Mos6502::reset(){
    A = X = Y = 0;
    SP = 0xFD;
    status = 0x24; // I=1, bit unused=1

    std::cout << "RESET chamado\n";

    uint8_t lo = memory->read(0xFFFC);
    uint8_t hi = memory->read(0xFFFD);
    PC = (hi << 8) | lo;
}


/* Modos de endereçamento */

uint16_t Mos6502::imm(){ // modo imediato
    return PC++;
}

uint16_t Mos6502::zp(){ // modo zero page
    uint8_t addr = memory->read(PC++);
    return addr;
}

uint16_t Mos6502::abs(){ // modo absoluto
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    return ((hi << 8) | lo);
}

uint16_t Mos6502::absx_no_cross() { // modo absoluto,X sem checagem de cruzamento de página
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    return ((hi << 8) | lo) + X;
}


uint16_t Mos6502::zpx() { // zero-page wrap
    uint8_t addr = memory->read(PC++);
    return (addr + X) & 0xFF; 
}

uint16_t Mos6502::zpy() { // zero-page wrap using Y
    uint8_t addr = memory->read(PC++);
    return (addr + Y) & 0xFF;
}

uint16_t Mos6502::absx() {
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    uint16_t base = (hi << 8) | lo;

    uint16_t addr = base + X;

    if((addr & 0xFF00) != (base & 0xFF00)){
        cycles++;
    }
    return addr;
}

uint16_t Mos6502::absy() {
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    uint16_t base = (hi << 8) | lo;

    uint16_t addr = base + Y;

    if((addr & 0xFF00) != (base & 0xFF00)){
        cycles++;
    }
    return addr;
}

uint16_t Mos6502::indx() {
    uint8_t zp_addr = (memory->read(PC++) + X) & 0xFF;
    uint8_t lo = memory->read(zp_addr);
    uint8_t hi = memory->read((zp_addr + 1) & 0xFF);
    return (hi << 8) | lo;
}

uint16_t Mos6502::indy() {
    uint8_t zp_addr = memory->read(PC++);
    uint8_t lo = memory->read(zp_addr);
    uint8_t hi = memory->read((zp_addr + 1) & 0xFF);
    
    uint16_t base = (hi << 8) | lo;
    uint16_t addr = base + Y;

    if((addr & 0xFF00) != (base & 0xFF00)){
        cycles++;
    }
    return addr;
}


static inline uint16_t addSigned8(uint16_t base, uint8_t disp){
    int8_t s = (int8_t)disp;
    return (uint16_t)(base + s);
}




void Mos6502::cpuClock(){
    uint8_t opcode = busca();

    switch(opcode){
        case 0xA9: { // LDA imediato
            uint16_t addr = imm();
            LDA(addr);
            cycles+=2;
            break;
        }

        case 0xA5: { // LDA zero page
            uint16_t addr = zp();
            LDA(addr);
            cycles+=3;
            break;
        }
    
        case 0xAD: { // LDA absoluto
            uint16_t addr = abs();
            LDA(addr);
            cycles+=4;
            break;
        }
        case 0xB5: { // LDA zero page,X
            uint16_t addr = zpx();
            LDA(addr);
            cycles+=4;
            break;
        }
        case 0xBD: { // LDA absolute,X
            uint16_t addr = absx();
            LDA(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0xB9: { // LDA absolute,Y
            uint16_t addr = absy();
            LDA(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0xA1: { // LDA (Indirect,X)
            uint16_t addr = indx();
            LDA(addr);
            cycles+=6;
            break;
        }
        case 0xB1: { // LDA (Indirect,Y)
            uint16_t addr = indy();
            LDA(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        // ASL - Arithmetic Shift Left 
        case 0x0A: { // ASL A (Accumulator)
            ASL_A();
            cycles+=2;
            break;
        }

        // ORA - OR with Accumulator
        case 0x09: { // ORA immediate
            uint16_t addr = imm();
            ORA(addr);
            cycles+=2;
            break;
        }
        case 0x05: { // ORA zero page
            uint16_t addr = zp();
            ORA(addr);
            cycles+=2;
            break;
        }
        case 0x15: { // ORA zero page,X
            uint16_t addr = zpx();
            ORA(addr);
            cycles+=3;
            break;
        }
        case 0x0D: { // ORA absolute
            uint16_t addr = abs();
            ORA(addr);
            cycles+=4;
            break;
        }
        case 0x1D: { // ORA absolute,X
            uint16_t addr = absx();
            ORA(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0x19: { // ORA absolute,Y
            uint16_t addr = absy();
            ORA(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0x01: { // ORA (Indirect,X)
            uint16_t addr = indx();
            ORA(addr);
            cycles+=6;
            break;
        }
        case 0x11: { // ORA (Indirect,Y)
            uint16_t addr = indy();
            ORA(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }
        case 0x06: { // ASL Zero Page
            uint16_t addr = zp();
            ASL(addr);
            cycles+=5;
            break;
        }
        case 0x16: { // ASL Zero Page,X
            uint16_t addr = zpx();
            ASL(addr);
            cycles+=6;
            break;
        }
        case 0x0E: { // ASL Absolute
            uint16_t addr = abs();
            ASL(addr);
            cycles+=6;
            break;
        }
        case 0x1E: { // ASL Absolute,X
            uint16_t addr = absx();
            ASL(addr);
            cycles+=7;
            break;
        }

        // LSR - Logical Shift Right
        case 0x4A: { // LSR A (Accumulator)
            LSR_A();
            cycles+=2;
            break;
        }
        case 0x46: { // LSR Zero Page
            uint16_t addr = zp();
            LSR(addr);
            cycles+=5;
            break;
        }
        case 0x56: { // LSR Zero Page,X
            uint16_t addr = zpx();
            LSR(addr);
            cycles+=6;
            break;
        }
        case 0x4E: { // LSR Absolute
            uint16_t addr = abs();
            LSR(addr);
            cycles+=6;
            break;
        }
        case 0x5E: { // LSR Absolute,X
            uint16_t addr = absx();
            LSR(addr);
            cycles+=7;
            break;
        }

        // ROL - Rotate Left
        case 0x2A: { // ROL A (Accumulator)
            ROL_A();
            cycles+=2;
            break;
        }
        case 0x26: { // ROL Zero Page
            uint16_t addr = zp();
            ROL(addr);
            cycles+=5;
            break;
        }
        case 0x36: { // ROL Zero Page,X
            uint16_t addr = zpx();
            ROL(addr);
            cycles+=6;
            break;
        }
        case 0x2E: { // ROL Absolute
            uint16_t addr = abs();
            ROL(addr);
            cycles+=6;
            break;
        }
        case 0x3E: { // ROL Absolute,X
            uint16_t addr = absx();
            ROL(addr);
            cycles+=7;
            break;
        }

        // ROR - Rotate Right
        case 0x6A: { // ROR A (Accumulator)
            ROR_A();
            cycles+=2;
            break;
        }
        case 0x66: { // ROR Zero Page
            uint16_t addr = zp();
            ROR(addr);
            cycles+=5;
            break;
        }
        case 0x76: { // ROR Zero Page,X
            uint16_t addr = zpx();
            ROR(addr);
            cycles+=6;
            break;
        }
        case 0x6E: { // ROR Absolute
            uint16_t addr = abs();
            ROR(addr);
            cycles+=6;
            break;
        }
        case 0x7E: { // ROR Absolute,X
            uint16_t addr = absx();
            ROR(addr);
            cycles+=7;
            break;
        }

        // BIT - Test Bits 
        case 0x24: { // BIT Zero Page
            uint16_t addr = zp();
            BIT(addr);
            cycles+=3;
            break;
        }
        case 0x2C: { // BIT Absolute
            uint16_t addr = abs();
            BIT(addr);
            cycles+=4;
            break;
        }

        // EOR - Exclusive OR with Accumulator
        case 0x49: { // EOR immediate
            uint16_t addr = imm();
            EOR(addr);
            cycles+=2;
            break;
        }
        case 0x45: { // EOR zero page
            uint16_t addr = zp();
            EOR(addr);
            cycles+=3;
            break;
        }
        case 0x55: { // EOR zero page,X
            uint16_t addr = zpx();
            EOR(addr);
            cycles+=4;
            break;
        }
        case 0x4D: { // EOR absolute
            uint16_t addr = abs();
            EOR(addr);
            cycles+=4;
            break;
        }
        case 0x5D: { // EOR absolute,X
            uint16_t addr = absx();
            EOR(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0x59: { // EOR absolute,Y
            uint16_t addr = absy();
            EOR(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0x41: { // EOR (Indirect,X)
            uint16_t addr = indx();
            EOR(addr);
            cycles+=6;
            break;
        }
        case 0x51: { // EOR (Indirect,Y)
            uint16_t addr = indy();
            EOR(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        // AND - Bitwise AND with Accumulator
        case 0x29: { // AND immediate
            uint16_t addr = imm();
            AND(addr);
            cycles+=2;
            break;
        }
        case 0x25: { // AND zero page
            uint16_t addr = zp();
            AND(addr);
            cycles+=2;
            break;
        }
        case 0x35: { // AND zero page,X
            uint16_t addr = zpx();
            AND(addr);
            cycles+=3;
            break;
        }
        case 0x2D: { // AND absolute
            uint16_t addr = abs();
            AND(addr);
            cycles+=4;
            break;
        }
        case 0x3D: { // AND absolute,X
            uint16_t addr = absx();
            AND(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0x39: { // AND absolute,Y
            uint16_t addr = absy();
            AND(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0x21: { // AND (Indirect,X)
            uint16_t addr = indx();
            AND(addr);
            cycles+=6;
            break;
        }
        case 0x31: { // AND (Indirect,Y)
            uint16_t addr = indy();
            AND(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        // DEC - Decrement Memory
        case 0xC6: { // DEC Zero Page
            uint16_t addr = zp();
            DEC(addr);
            cycles+=5;
            break;
        }
        case 0xD6: { // DEC Zero Page,X
            uint16_t addr = zpx();
            DEC(addr);
            cycles+=6;
            break;
        }
        case 0xCE: { // DEC Absolute
            uint16_t addr = abs();
            DEC(addr);
            cycles+=6;
            break;
        }
        case 0xDE: { // DEC Absolute,X
            uint16_t addr = absx();
            DEC(addr);
            cycles+=7;
            break;
        }

        // INC - Increment Memory
        case 0xE6: { // INC Zero Page
            uint16_t addr = zp();
            INC(addr);
            cycles+=5;
            break;
        }
        case 0xF6: { // INC Zero Page,X
            uint16_t addr = zpx();
            INC(addr);
            cycles+=6;
            break;
        }
        case 0xEE: { // INC Absolute
            uint16_t addr = abs();
            INC(addr);
            cycles+=6;
            break;
        }
        case 0xFE: { // INC Absolute,X
            uint16_t addr = absx();
            INC(addr);
            cycles+=7;
            break;
        }

        // CMP - Compare Accumulator
        case 0xC9: { // CMP immediate
            uint16_t addr = imm();
            CMP(addr);
            cycles+=2;
            break;
        }
        case 0xC5: { // CMP zero page
            uint16_t addr = zp();
            CMP(addr);
            cycles+=3;
            break;
        }
        case 0xD5: { // CMP zero page,X
            uint16_t addr = zpx();
            CMP(addr);
            cycles+=4;
            break;
        }
        case 0xCD: { // CMP absolute
            uint16_t addr = abs();
            CMP(addr);
            cycles+=4;
            break;
        }
        case 0xDD: { // CMP absolute,X
            uint16_t addr = absx();
            CMP(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0xD9: { // CMP absolute,Y
            uint16_t addr = absy();
            CMP(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0xC1: { // CMP (Indirect,X)
            uint16_t addr = indx();
            CMP(addr);
            cycles+=6;
            break;
        }
        case 0xD1: { // CMP (Indirect,Y)
            uint16_t addr = indy();
            CMP(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        /* CPX - Compare X register */
        case 0xE0: { // CPX immediate
            uint16_t addr = imm();
            CPX(addr);
            cycles+=2;
            break;
        }
        case 0xE4: { // CPX zero page
            uint16_t addr = zp();
            CPX(addr);
            cycles+=3;
            break;
        }
        case 0xEC: { // CPX absolute
            uint16_t addr = abs();
            CPX(addr);
            cycles+=4;
            break;
        }

        /* CPY - Compare Y register */
        case 0xC0: { // CPY immediate
            uint16_t addr = imm();
            CPY(addr);
            cycles+=2;
            break;
        }
        case 0xC4: { // CPY zero page
            uint16_t addr = zp();
            CPY(addr);
            cycles+=3;
            break;
        }
        case 0xCC: { // CPY absolute
            uint16_t addr = abs();
            CPY(addr);
            cycles+=4;
            break;
        }

        // ADC - Add with Carry
        case 0x69: { // ADC imediato
            uint16_t addr = imm();
            ADC(addr);
            cycles+=2;
            break;
        }

        case 0x65: { // ADC zero page
            uint16_t addr = zp();
            ADC(addr);
            cycles+=3;
            break;
        }

        case 0x75: { // ADC zero page,X
            uint16_t addr = zpx();
            ADC(addr);
            cycles+=4;
            break;
        }

        case 0x6D: { // ADC absoluto
            uint16_t addr = abs();
            ADC(addr);
            cycles+=4;
            break;
        }

        case 0x7D: { // ADC absoluto,X
            uint16_t addr = absx();
            ADC(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }

        case 0x79: { // ADC absoluto,Y
            uint16_t addr = absy();
            ADC(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }

        case 0x61: { // ADC indireto,X
            uint16_t addr = indx();
            ADC(addr);
            cycles+=6;
            break;
        }

        case 0x71: { // ADC indireto,Y
            uint16_t addr = indy();
            ADC(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        // SBC - Subtract with Carry
        case 0xE9: { // SBC immediate
            uint16_t addr = imm();
            SBC(addr);
            cycles+=2;
            break;
        }
        case 0xE5: { // SBC zero page
            uint16_t addr = zp();
            SBC(addr);
            cycles+=3;
            break;
        }
        case 0xF5: { // SBC zero page,X
            uint16_t addr = zpx();
            SBC(addr);
            cycles+=4;
            break;
        }
        case 0xED: { // SBC absolute
            uint16_t addr = abs();
            SBC(addr);
            cycles+=4;
            break;
        }
        case 0xFD: { // SBC absolute,X
            uint16_t addr = absx();
            SBC(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }
        case 0xF9: { // SBC absolute,Y
            uint16_t addr = absy();
            SBC(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }
        case 0xE1: { // SBC (Indirect,X)
            uint16_t addr = indx();
            SBC(addr);
            cycles+=6;
            break;
        }
        case 0xF1: { // SBC (Indirect,Y)
            uint16_t addr = indy();
            SBC(addr);
            cycles+=5; // +1 se cruzar página (já tratado em indy)
            break;
        }

        // STA - Store Accumulator
        case 0x85: { // STA zero page
            uint16_t addr = zp();
            STA(addr);
            cycles+=3;
            break;
        }

        case 0x9D: { // STA absolute,X
            uint16_t addr = absx_no_cross();
            STA(addr);
            cycles+=4;
            break;
        }

        case 0x81: { // STA (Indirect,X)
            uint16_t addr = indx();
            STA(addr);
            cycles+=6;
            break;
        }

        case 0x95: { // STA zero page,X
            uint16_t addr = zpx();
            STA(addr);
            cycles+=4;
            break;
        }

        case 0x91: { // STA (Indirect,Y)
            uint16_t addr = indy();
            STA(addr);
            cycles+=6;
            break;
        }

        case 0x99: { // STA absolute,Y
            uint16_t addr = absy();
            STA(addr);
            cycles+=5;
            break;
        }

        case 0x8D: { // STA absolute
            uint16_t addr = abs();
            STA(addr);
            cycles+=4;
            break;
        }

        // STX - Store X Register
        case 0x86: { // STX zero page
            uint16_t addr = zp();
            STX(addr);
            cycles+=3;
            break;
        }
        case 0x96: { // STX zero page,Y
            uint16_t addr = zpy();
            STX(addr);
            cycles+=4;
            break;
        }
        case 0x8E: { // STX absolute
            uint16_t addr = abs();
            STX(addr);
            cycles+=4;
            break;
        }

        // STY - Store Y Register
        case 0x84: { // STY zero page
            uint16_t addr = zp();
            STY(addr);
            cycles+=3;
            break;
        }
        case 0x94: { // STY zero page,X
            uint16_t addr = zpx();
            STY(addr);
            cycles+=4;
            break;
        }
        case 0x8C: { // STY absolute
            uint16_t addr = abs();
            STY(addr);
            cycles+=4;
            break;
        }

        /* LDX - Load X */
        case 0xA2: { // LDX immediate
            uint16_t addr = imm();
            LDX(addr);
            cycles+=2;
            break;
        }

        case 0xA6: { // LDX zero page
            uint16_t addr = zp();
            LDX(addr);
            cycles+=3;
            break;
        }

        case 0xAE: { // LDX absolute
            uint16_t addr = abs();
            LDX(addr);
            cycles+=4;
            break;
        }

        case 0xB6: { // LDX zero page,Y
            uint16_t addr = zpy();
            LDX(addr);
            cycles+=4;
            break;
        }

        case 0xBE: { // LDX absolute,Y
            uint16_t addr = absy();
            LDX(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absy)
            break;
        }

        // LDY - Load Y
        case 0xA0: { // LDY immediate
            uint16_t addr = imm();
            LDY(addr);
            cycles+=2;
            break;
        }

        case 0xA4: { // LDY zero page
            uint16_t addr = zp();
            LDY(addr);
            cycles+=3;
            break;
        }

        case 0xAC: { // LDY absolute
            uint16_t addr = abs();
            LDY(addr);
            cycles+=4;
            break;
        }
        case 0xB4: { // LDY zero page,X
            uint16_t addr = zpx();
            LDY(addr);
            cycles+=4;
            break;
        }
        case 0xBC: { // LDY absolute,X
            uint16_t addr = absx();
            LDY(addr);
            cycles+=4; // +1 se cruzar página (já tratado em absx)
            break;
        }

        
        



        // Branch Instructions - relative addressing
        case 0x10: { // BPL (Branch on Plus) N == 0
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(!getFlag(NEGATIVE)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0x30: { // BMI (Branch on Minus) N == 1
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(getFlag(NEGATIVE)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0x50: { // BVC (Branch on Overflow Clear) V == 0
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(!getFlag(OVERFLOW)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0x70: { // BVS (Branch on Overflow Set) V == 1
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(getFlag(OVERFLOW)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0x90: { // BCC (Branch on Carry Clear) C == 0
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(!getFlag(CARRY)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0xB0: { // BCS (Branch on Carry Set) C == 1
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(getFlag(CARRY)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0xD0: { // BNE (Branch on Not Equal) Z == 0
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(!getFlag(ZERO)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }
        case 0xF0: { // BEQ (Branch on Equal) Z == 1
            uint8_t disp = memory->read(PC++);
            cycles+=2;
            if(getFlag(ZERO)){
                uint16_t oldPC = PC;
                PC = addSigned8(PC, disp);
                cycles++;
                if((oldPC & 0xFF00) != (PC & 0xFF00)){
                    cycles++;
                }
            }
            break;
        }





        case 0x00: { // BRK - Force Interrupt
            // On BRK, the CPU pushes PC+1 and the status with B flag set,
            // sets Interrupt Disable, then loads the IRQ/BRK vector at $FFFE/$FFFF.
            uint16_t return_addr = PC + 1; // PC currently points to the next byte after opcode

            // Push PCH
            memory->write(0x0100 + SP, (uint8_t)((return_addr >> 8) & 0xFF));
            SP--;
            // Push PCL
            memory->write(0x0100 + SP, (uint8_t)(return_addr & 0xFF));
            SP--;
            // Push status with B set (and preserve UNUSED bit)
            uint8_t pushedP = status | BREAK;
            memory->write(0x0100 + SP, pushedP);
            SP--;

            // Set Interrupt Disable
            setFlag(INTERRUPT_DISABLE, true);

            // Load vector from $FFFE/$FFFF
            uint8_t lo = memory->read(0xFFFE);
            uint8_t hi = memory->read(0xFFFF);
            PC = (uint16_t)((hi << 8) | lo);
            cycles+=7;
            break;
        }

        case 0x40: { // RTI - Return from Interrupt
            // Pull processor status
            status = memory->read(0x0100 + ++SP);
            status &= ~BREAK;   // B sempre limpo após RTI
            status |= UNUSED;   // Bit 5 sempre ligado

            // Pull PC
            uint8_t lo = memory->read(0x0100 + ++SP);
            uint8_t hi = memory->read(0x0100 + ++SP);
            PC = (hi << 8) | lo;
            cycles+=6;
            break;
        }


        case 0xF8: { // SED - Set Decimal Mode
            setFlag(DECIMAL_MODE, true);
            break;
        }
        case 0xD8: { // CLD - Clear Decimal Mode
            setFlag(DECIMAL_MODE, false);
            break;
        }

        case 0x4C: { // JMP Absolute
            PC = abs();
            cycles+=3;
            break;
        }

        case 0x6C: { // JMP Indirect (with 6502 page-wrap bug)
            uint16_t ptr = abs();
            uint8_t lo = memory->read(ptr);
            uint8_t hi = memory->read((ptr & 0xFF00) | ((ptr + 1) & 0x00FF));
            PC = (uint16_t)((hi << 8) | lo);
            cycles+=5;
            break;
        }

        case 0x20: { // JSR (Jump to Sub Routine)
            uint16_t addr = abs();
            uint16_t ret = PC - 1;

            memory->write(0x0100 + SP--, (ret >> 8) & 0xFF);
            memory->write(0x0100 + SP--, ret & 0xFF);

            PC = addr;
            cycles+=6;
            break;
        }

        case 0xEA: { // NOP
            cycles+=2;
            break;
        }

        // Flags básicos
        case 0x18: setFlag(CARRY, false); cycles+=2; break; // CLC
        case 0x38: setFlag(CARRY, true);  cycles+=2; break; // SEC
        case 0x58: setFlag(INTERRUPT_DISABLE, false); cycles+=2; break; // CLI
        case 0x78: setFlag(INTERRUPT_DISABLE, true);  cycles+=2; break; // SEI
        case 0xB8: setFlag(OVERFLOW, false); cycles+=2; break; // CLV
        // Incrementos / Decrementos
        case 0xE8: X++; updateZN(X); cycles+=2; break; // INX
        case 0xCA: X--; updateZN(X); cycles+=2; break; // DEX
        case 0xC8: Y++; updateZN(Y); cycles+=2; break; // INY
        case 0x88: Y--; updateZN(Y); cycles+=2; break; // DEY
        // Transferências
        case 0xAA: X = A; updateZN(X); cycles+=2; break; // TAX
        case 0x8A: A = X; updateZN(A); cycles+=2; break; // TXA
        case 0xA8: Y = A; updateZN(Y); cycles+=2; break; // TAY
        case 0x98: A = Y; updateZN(A); cycles+=2; break; // TYA

        case 0x60: { //  RTS (ReTurn from Subroutine)
            uint8_t lo = pop();
            uint8_t hi = pop();
            PC = ((hi << 8) | lo) + 1;
            cycles+=6;
            break;
        }

        case 0x48: { // PHA (push accumulator)
            push(A);
            cycles+=3;
            break;
        }

        case 0x68: { // PLA (pulll accumulator)
            A = memory->read(0x0100 + ++SP);
            updateZN(A);
            cycles+=4;
            break;
        }

        case 0x9A: { // TXS (Transfer X to Stack Pointer)
            SP = X;
            cycles+=2;
            break;
        }

        case 0xBA: { // TSX (Transfer Stack Pointer to X)
            X = SP;
            updateZN(X);
            cycles+=2;
            break;
        }

        case 0x08: { // PHP (Push Processor Status)
            uint8_t pushed = status | BREAK | UNUSED;
            memory->write(0x0100 + SP--, pushed);
            cycles+=3;
            break;
        }

        case 0x28: { // PLP (Pull Processor Status)
            status = memory->read(0x0100 + ++SP);
            status &= ~BREAK;
            status |= UNUSED;
            cycles+=4;
            break;
        }



        default:
            std::cerr << "Opcode desconhecido: $" << std::hex << (int)opcode << std::endl;
            dumpState();
            exit(1);
    }
}

void Mos6502::setFlag(FLAGS flag, bool value){
    if(value){
        status |= flag; // or bit a bit (liga o bit 1)
    } else {
        status &= ~flag; // and bit a bit com negacao (desliga o bit 1)
    }
}

bool Mos6502::getFlag(FLAGS flag) const{
    return (status & flag) != 0;
}

void Mos6502::dumpState() const {
    printf("A=%02X X=%02X Y=%02X SP=%02X PC=%04X P=%02X  ",
           A, X, Y, SP, PC, status);

    printf("[N=%d V=%d - B=%d D=%d I=%d Z=%d C=%d]\n",
           (status >> 7) & 1, (status >> 6) & 1, (status >> 4) & 1,
           (status >> 3) & 1, (status >> 2) & 1, (status >> 1) & 1,
           status & 1);
}

void Mos6502::push(uint8_t value) {
    memory->write(0x0100 + SP, value);
    SP--;
}

uint8_t Mos6502::pop() {
    SP++;
    return memory->read(0x0100 + SP);
}

uint16_t Mos6502::read_word(uint16_t address) {
    uint8_t lo = memory->read(address);
    uint8_t hi = memory->read(address + 1);
    return (hi << 8) | lo;
}
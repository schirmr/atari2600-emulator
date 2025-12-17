#include "mos6502r.hpp" // Processador MOS 6502 Reduzido ou 6507
#include <iostream>

Mos6502::Mos6502(Memory* mem){
    this->memory = mem;
}

uint8_t Mos6502::busca(){
    return memory->read(PC++);
}

void Mos6502::LDA(uint16_t address){ // Load Accumulator
    A = memory->read(address);
    updateZN(A);
}

void Mos6502::updateZN(uint8_t value){
    setFlag(ZERO, value == 0x00);
    setFlag(NEGATIVE, (value & 0x80) != 0);
}

void Mos6502::ADC(uint16_t address){
    uint8_t m = memory->read(address);
    uint16_t carry_in = getFlag(CARRY) ? 1 : 0;

    // Binário básico para calcular V conforme 6502 (mesmo em decimal)
    uint16_t bin_sum = (uint16_t)A + (uint16_t)m + carry_in;
    uint8_t bin_result = (uint8_t)(bin_sum & 0xFF);
    bool overflow = (~(A ^ m) & (A ^ bin_result) & 0x80) != 0; // V é baseado no somatório binário

    if(getFlag(DECIMAL_MODE)){
        // Ajuste BCD (modo decimal): soma 4 bits baixo e alto separadamente
        uint8_t al = (A & 0x0F) + (m & 0x0F) + (uint8_t)carry_in;
        uint8_t ah = (A >> 4) + (m >> 4);

        if(al > 9){
            al += 6; // correção decimal
            ah += 1;
        }
        if(ah > 9){
            ah += 6; // correção decimal
        }

        uint16_t dec = ((uint16_t)ah << 4) | (al & 0x0F);
        // Carry em BCD: se ah passou de 15 após correção
        bool dec_carry = ah > 15;

        setFlag(CARRY, dec_carry);
        setFlag(OVERFLOW, overflow); // 6502 define V pelo somatório binário mesmo em decimal

        A = (uint8_t)(dec & 0xFF);
        updateZN(A);
    } else {
        // Modo binário normal
        setFlag(CARRY, bin_sum > 0xFF);
        setFlag(OVERFLOW, overflow);
        A = bin_result;
        updateZN(A);
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

void Mos6502::BIT(uint16_t address){
    uint8_t m = memory->read(address);
    // Z flag: (A & M) == 0
    setFlag(ZERO, (uint8_t)(A & m) == 0);
    // N and V reflect bits 7 and 6 of memory
    setFlag(NEGATIVE, (m & 0x80) != 0);
    setFlag(OVERFLOW, (m & 0x40) != 0);
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

uint16_t Mos6502::zpx() { // zero-page wrap
    uint8_t addr = memory->read(PC++);
    return (addr + X) & 0xFF; 
}

uint16_t Mos6502::absx() {
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    return ((hi << 8) | lo) + X;
}

uint16_t Mos6502::absy() {
    uint8_t lo = memory->read(PC++);
    uint8_t hi = memory->read(PC++);
    return ((hi << 8) | lo) + Y;
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
    return ((hi << 8) | lo) + Y;
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
            break;
        }

        // ASL - Arithmetic Shift Left 
        case 0x0A: { // ASL A (Accumulator)
            ASL_A();
            break;
        }
        case 0x06: { // ASL Zero Page
            uint16_t addr = zp();
            ASL(addr);
            break;
        }
        case 0x16: { // ASL Zero Page,X
            uint16_t addr = zpx();
            ASL(addr);
            break;
        }
        case 0x0E: { // ASL Absolute
            uint16_t addr = abs();
            ASL(addr);
            break;
        }
        case 0x1E: { // ASL Absolute,X
            uint16_t addr = absx();
            ASL(addr);
            break;
        }

        // BIT - Test Bits 
        case 0x24: { // BIT Zero Page
            uint16_t addr = zp();
            BIT(addr);
            break;
        }
        case 0x2C: { // BIT Absolute
            uint16_t addr = abs();
            BIT(addr);
            break;
        }

        // Branch Instructions - relative addressing
        case 0x10: { // BPL (Branch on Plus) N == 0
            uint8_t disp = memory->read(PC++);
            if(!getFlag(NEGATIVE)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0x30: { // BMI (Branch on Minus) N == 1
            uint8_t disp = memory->read(PC++);
            if(getFlag(NEGATIVE)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0x50: { // BVC (Branch on Overflow Clear) V == 0
            uint8_t disp = memory->read(PC++);
            if(!getFlag(OVERFLOW)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0x70: { // BVS (Branch on Overflow Set) V == 1
            uint8_t disp = memory->read(PC++);
            if(getFlag(OVERFLOW)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0x90: { // BCC (Branch on Carry Clear) C == 0
            uint8_t disp = memory->read(PC++);
            if(!getFlag(CARRY)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0xB0: { // BCS (Branch on Carry Set) C == 1
            uint8_t disp = memory->read(PC++);
            if(getFlag(CARRY)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0xD0: { // BNE (Branch on Not Equal) Z == 0
            uint8_t disp = memory->read(PC++);
            if(!getFlag(ZERO)){
                PC = addSigned8(PC, disp);
            }
            break;
        }
        case 0xF0: { // BEQ (Branch on Equal) Z == 1
            uint8_t disp = memory->read(PC++);
            if(getFlag(ZERO)){
                PC = addSigned8(PC, disp);
            }
            break;
        }

        // AND - Bitwise AND with Accumulator
        case 0x29: { // AND immediate
            uint16_t addr = imm();
            AND(addr);
            break;
        }
        case 0x25: { // AND zero page
            uint16_t addr = zp();
            AND(addr);
            break;
        }
        case 0x35: { // AND zero page,X
            uint16_t addr = zpx();
            AND(addr);
            break;
        }
        case 0x2D: { // AND absolute
            uint16_t addr = abs();
            AND(addr);
            break;
        }
        case 0x3D: { // AND absolute,X
            uint16_t addr = absx();
            AND(addr);
            break;
        }
        case 0x39: { // AND absolute,Y
            uint16_t addr = absy();
            AND(addr);
            break;
        }
        case 0x21: { // AND (Indirect,X)
            uint16_t addr = indx();
            AND(addr);
            break;
        }
        case 0x31: { // AND (Indirect,Y)
            uint16_t addr = indy();
            AND(addr);
            break;
        }

        // CMP - Compare Accumulator
        case 0xC9: { // CMP immediate
            uint16_t addr = imm();
            CMP(addr);
            break;
        }
        case 0xC5: { // CMP zero page
            uint16_t addr = zp();
            CMP(addr);
            break;
        }
        case 0xD5: { // CMP zero page,X
            uint16_t addr = zpx();
            CMP(addr);
            break;
        }
        case 0xCD: { // CMP absolute
            uint16_t addr = abs();
            CMP(addr);
            break;
        }
        case 0xDD: { // CMP absolute,X
            uint16_t addr = absx();
            CMP(addr);
            break;
        }
        case 0xD9: { // CMP absolute,Y
            uint16_t addr = absy();
            CMP(addr);
            break;
        }
        case 0xC1: { // CMP (Indirect,X)
            uint16_t addr = indx();
            CMP(addr);
            break;
        }
        case 0xD1: { // CMP (Indirect,Y)
            uint16_t addr = indy();
            CMP(addr);
            break;
        }

        /* CPX - Compare X register */
        case 0xE0: { // CPX immediate
            uint16_t addr = imm();
            CPX(addr);
            break;
        }
        case 0xE4: { // CPX zero page
            uint16_t addr = zp();
            CPX(addr);
            break;
        }
        case 0xEC: { // CPX absolute
            uint16_t addr = abs();
            CPX(addr);
            break;
        }
        /* CPY - Compare Y register */
        case 0xC0: { // CPY immediate
            uint16_t addr = imm();
            CPY(addr);
            break;
        }
        case 0xC4: { // CPY zero page
            uint16_t addr = zp();
            CPY(addr);
            break;
        }
        case 0xCC: { // CPY absolute
            uint16_t addr = abs();
            CPY(addr);
            break;
        }

        // ADC - Add with Carry
        case 0x69: { // ADC imediato
            uint16_t addr = imm();
            ADC(addr);
            break;
        }

        case 0x65: { // ADC zero page
            uint16_t addr = zp();
            ADC(addr);
            break;
        }

        case 0x75: { // ADC zero page,X
            uint16_t addr = zpx();
            ADC(addr);
            break;
        }

        case 0x6D: { // ADC absoluto
            uint16_t addr = abs();
            ADC(addr);
            break;
        }

        case 0x7D: { // ADC absoluto,X
            uint16_t addr = absx();
            ADC(addr);
            break;
        }

        case 0x79: { // ADC absoluto,Y
            uint16_t addr = absy();
            ADC(addr);
            break;
        }

        case 0x61: { // ADC indireto,X
            uint16_t addr = indx();
            ADC(addr);
            break;
        }

        case 0x71: { // ADC indireto,Y
            uint16_t addr = indy();
            ADC(addr);
            break;
        }

        // STA - Store Accumulator
        case 0x85: { // STA zero page
            uint16_t addr = zp();
            STA(addr);
            break;
        }

        case 0x8D: { // STA absolute
            uint16_t addr = abs();
            STA(addr);
            break;
        }

        /* LDX - Load X */
        case 0xA2: { // LDX immediate
            uint16_t addr = imm();
            LDX(addr);
            break;
        }

        case 0xA6: { // LDX zero page
            uint16_t addr = zp();
            LDX(addr);
            break;
        }

        case 0xAE: { // LDX absolute
            uint16_t addr = abs();
            LDX(addr);
            break;
        }

        // LDY - Load Y
        case 0xA0: { // LDY immediate
            uint16_t addr = imm();
            LDY(addr);
            break;
        }

        case 0xA4: { // LDY zero page
            uint16_t addr = zp();
            LDY(addr);
            break;
        }

        case 0xAC: { // LDY absolute
            uint16_t addr = abs();
            LDY(addr);
            break;
        }

        case 0xA5: { // LDA zero page
            uint16_t addr = zp();
            LDA(addr);
            break;
        }
    
        case 0xAD: { // LDA absoluto
            uint16_t addr = abs();
            LDA(addr);
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
            break;
        }

        case 0x20: { // JSR (Jump to Sub Routine)
            uint16_t addr = abs();
            uint16_t ret = PC - 1;

            memory->write(0x0100 + SP--, (ret >> 8) & 0xFF);
            memory->write(0x0100 + SP--, ret & 0xFF);

            PC = addr;
            break;
        }

        case 0xEA: { // NOP
            break;
        }

        // Flags básicos
        case 0x18: setFlag(CARRY, false); break; // CLC
        case 0x38: setFlag(CARRY, true);  break; // SEC
        case 0x58: setFlag(INTERRUPT_DISABLE, false); break; // CLI
        case 0x78: setFlag(INTERRUPT_DISABLE, true);  break; // SEI
        case 0xB8: setFlag(OVERFLOW, false); break; // CLV
        // Incrementos / Decrementos
        case 0xE8: X++; updateZN(X); break; // INX
        case 0xCA: X--; updateZN(X); break; // DEX
        case 0xC8: Y++; updateZN(Y); break; // INY
        case 0x88: Y--; updateZN(Y); break; // DEY
        // Transferências
        case 0xAA: X = A; updateZN(X); break; // TAX
        case 0x8A: A = X; updateZN(A); break; // TXA
        case 0xA8: Y = A; updateZN(Y); break; // TAY
        case 0x98: A = Y; updateZN(A); break; // TYA

        case 0x60: { //  RTS (ReTurn from Subroutine)
            uint8_t lo = memory->read(0x0100 + ++SP);
            uint8_t hi = memory->read(0x0100 + ++SP);
            PC = ((hi << 8) | lo) + 1;
            break;
        }

        case 0x48: { // PHA (push accumulator)
            memory->write(0x0100 + SP--, A);
            break;
        }

        case 0x68: { // PLA (pulll accumulator)
            A = memory->read(0x0100 + ++SP);
            updateZN(A);
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

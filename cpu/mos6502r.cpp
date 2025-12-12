#include "mos6502r.hpp"
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
    uint16_t sum = (uint16_t)A + (uint16_t)m + (getFlag(CARRY) ? 1 : 0);

    // Carry flag (1 or 0)
    setFlag(CARRY, sum > 0xFF);

    uint8_t result = (uint8_t)(sum & 0xFF); // Guarda apenas os 8 bits baixos

    bool overflow = (~(A ^ m) & (A ^ result) & 0x80) != 0; // ~^(XNOR) p/ detectar overflow (0x80 = bit de sinal = 128)
    setFlag(OVERFLOW, overflow);

    A = result;
    updateZN(A);
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





void Mos6502::cpuClock(){
    uint8_t opcode = busca();
    
    switch(opcode){
        case 0xA9: { // LDA imediato
            uint16_t addr = imm();
            LDA(addr);
            break;
        }

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

        case 0x6D: { // ADC absoluto
            uint16_t addr = abs();
            ADC(addr);
            break;
        }

        /* STA - Store Accumulator */
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

        /* LDY - Load Y */
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
        
        case 0x00: // BRK
            std::cout << "CPU: BRK encontrado, parando execução." << std::endl;
            exit(0);

        default:
            std::cerr << "Opcode desconhecido: $" << std::hex << (int)opcode << std::endl;
            break;
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

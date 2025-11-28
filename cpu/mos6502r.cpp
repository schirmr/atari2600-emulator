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
    setFlag(ZERO, A == 0x00);
    setFlag(NEGATIVE, A & 0x80);
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

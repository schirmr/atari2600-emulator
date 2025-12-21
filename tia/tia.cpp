#include "tia.hpp"
#include <iostream>

Tia::Tia() {
    reset();
}

void Tia::reset() {
    for(int i=0; i<64; i++) registers[i] = 0;
}

uint8_t Tia::read(uint16_t addr) {
    uint8_t reg = addr & 0x3F;

    if (reg >= 0x30 && reg <= 0x3D) {
        // retornar colisao e inputs futuramente
        return 0x00; 
    }

    return registers[reg]; 
}

void Tia::write(uint16_t addr, uint8_t val) {
    uint8_t reg = addr & 0x3F;
    
    registers[reg] = val;

    if(reg == 0x09) std::cout << "Cor de fundo: " << std::hex << (int)val << "\n"; // debug das cores mudando

    if(reg == 0x02) {
        // logica do WSYNC (futuramente)
    }
}
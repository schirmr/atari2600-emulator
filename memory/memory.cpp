#include <iostream>
#include <cstdio>
#include "memory.hpp"

Memory::Memory() {
    // Inicia memória com 0
    for (int i = 0; i < 65536; i++) {
        mem[i] = 0x00;
    }
}

uint8_t Memory::read(uint16_t addr) const {
    return mem[addr];
}

void Memory::write(uint16_t addr, uint8_t data) {
    mem[addr] = data;
}

void Memory::dump(uint16_t start, uint16_t end) const{
    for(uint16_t addr = start; addr <= end; addr++){
        printf("%04X: %02X\n", addr, mem[addr]);
    }
}

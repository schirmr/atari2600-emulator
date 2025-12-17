#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include "memory.hpp"

Memory::Memory() {
    // Inicia memória com 0
    for (int i = 0; i < 65536; i++) {
        mem[i] = 0x00;
    }
}

uint8_t Memory::read(uint16_t addr) const {
    if(addr < 0x80){
        return 0x00;
    }

    if(addr >= 0xF000 && romSize > 0){
        return rom[(addr - 0xF000) % romSize]; // Acesso à ROM mapeada em $F000-$FFFF
    }
    return mem[addr];
}

void Memory::write(uint16_t addr, uint8_t data) {
    if (addr < 0x80) {
        return; // Memória de somente leitura ou reservada
    }

    if(addr >= 0xF000){
        return; // ROM mapeada, não pode escrever
    }
    
    mem[addr] = data;
}

void Memory::dump(uint16_t start, uint16_t end) const{
    for(uint16_t addr = start; addr <= end; addr++){
        printf("%04X: %02X\n", addr, mem[addr]);
    }
}

void Memory::loadROM(const std::string& path){
    std::ifstream file(path, std::ios::binary);
    if(!file){
        std::cerr << "Erro ao abrir ROM\n";
        return;
    }
    
    file.read((char*)rom, sizeof(rom));
    romSize = (uint16_t)file.gcount(); // tamanho real lido

    std::cout << "ROM carregada: " << romSize << " bytes\n";
}

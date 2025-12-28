#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include "memory.hpp"
#include "riot.hpp"

Memory::Memory() {
    std::memset(rom, 0, sizeof(rom)); // evitar lixos
    romSize = 0;
    
    riot.reset();
}

uint8_t Memory::read(uint16_t addr) const {
    // 1. Checa a ROM
    if (addr & 0xF000) { // acesso a rom ($F000-$FFFF)
        return rom[addr & 0x0FFF]; // offset 0-4095
    }
    // 2. TIA read
    if ((addr & 0x0080) == 0) { // TIA read ($0000-$007F)
        return const_cast<Tia&>(tia).read(addr); // read do TIA pode limpar flags
    }
    // 3. Ram e stack
    if ((addr & 0x0280) == 0x0080) {  // Acesso à RAM e Stack ($0080-$00FF e $0180-$01FF)
        return riot.ram[addr & 0x007F]; // map para 0-127 dentro do array da RAM
    }
    // 4. Riot I/O e Timer
    if ((addr & 0x0280) == 0x0280) {  // leitura registradores PIA (Timer/Ports) - $0280-$0297
        return const_cast<Riot&>(riot).ioRead(addr); 
    }

    // acesso ao TIA
    // por enquanto retorna 0, ler colisão depois
    return 0x00;
}

void Memory::write(uint16_t addr, uint8_t data) {
    if((addr & 0x0080) == 0) { // Escrita no TIA ($0000-$007F)
        tia.write(addr, data);

        if (tia.isWSYNCActive()) {
            while (tia.isWSYNCActive()) {
                // TIA roda 3 clocks por ciclo da CPU
                tia.clock();
                tia.clock();
                tia.clock();
                riot.step(1);
            }
        }
        return;
    }

    if ((addr & 0x0280) == 0x0080) {     // Escrita na RAM e Stack ($0080-$00FF e $0180-$01FF)
        riot.ram[addr & 0x007F] = data;
        return;
    }

    if ((addr & 0x0280) == 0x0280) {  // escrita registradores PIA (Timer/Ports) - $0280-$0297
        riot.ioWrite(addr, data);
        return;
    }

    // desenho da tela
    if ((addr & 0xF000) == 0) { // Escrita no TIA ($0000-$007F)
        // logica de escrita do TIA
        return;
    }
}

void Memory::dump(uint16_t start, uint16_t end) const{
    for(uint16_t addr = start; addr <= end; addr++){
        printf("%04X: %02X\n", addr, read(addr));
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

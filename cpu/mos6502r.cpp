#include "mos6502r.hpp"
#include <iostream>

Mos6502::Mos6502(Memory* mem){
    this->memory = mem;
}

uint8_t Mos6502::busca(){
    return memory->read(PC++);
}

void Mos6502::cpuClock(){
    uint8_t opcode = busca();
    switch(opcode){
        case 0xA9: { // LDA imediato
            uint8_t value = busca();
            A = value;
            std::cout << "LDA 0x" << std::hex << (int)value << std::endl;
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
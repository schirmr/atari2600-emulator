#include <iostream> // entrada e saída padrão 
#include <fstream> // leitura e escrita de arquivos 
#include <vector> // vetor dinâmico 
#include <cstdint> // larguras de int fixos 

//#include "memory.h"
//#include "cpu/mos6502r.hpp"

uint8_t memory[65536]; // 64KB de memória 

uint16_t PC = 0x0000; // registrador de contador de programa (program counter)
uint8_t A = 0x00;   // registrador acumulador (acumulator)

// busca 1 byte e incrementa o PC
uint8_t busca(){
    return memory[PC++];
}

void cpu_clock(){
    uint8_t opcode = busca();

    switch(opcode){
        case 0xA9: // LDA imediato
            A = busca();
            std::cout << "LDA imediato: " << std::hex << (int)A << std::endl; // buscas printadas em hexadecimal
            break;
        case 0x00: // BREK (break)
            std::cout << "Programa finalizado." << std::endl;
            exit(0);
            break;
        default:
            std::cerr << "Opcode desconhecido: " << std::hex << (int)opcode << std::endl;
            break;
    }
}

int main(){
    memory[0x0000] = 0xA9; // LDA imediato
    memory[0x0001] = 0x42; // valor a ser carregado no acumulador

    while(true){
        cpu_clock();
    }

    return 0;
}
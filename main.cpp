#include <iostream>
#include <cstdlib>
#include "./memory/memory.hpp"
#include "./cpu/mos6502r.hpp"

int main() {
    Memory memory;
    Mos6502 cpu(&memory);

    memory.loadROM("./tests/space_invaders.a26");
    cpu.reset();

    const char* venv = std::getenv("VERBOSE");
    cpu.verbose = (venv && venv[0] != '0');

    std::cout << "Starting Space Invaders emulation...\n";
    std::cout << "PC Inicial: " << std::hex << cpu.PC << "\n";

    for(long i = 0; i < 20000; i++){ // roda 20000 instruções do jogo (caso na o trave)
        cpu.cpuClock();

        memory.step(3); // rodando cada instrução gastando uma media de 3 clocks
        if(cpu.verbose){
            cpu.dumpState();
        }

    }

    return 0;
}

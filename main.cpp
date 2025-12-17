#include <iostream>
#include "./memory/memory.hpp"
#include "./cpu/mos6502r.hpp"

int main() {
    Memory memory;
    Mos6502 cpu(&memory);

    memory.loadROM("./tests/space_invaders.a26");
    cpu.reset();

    std::cout << "Starting Space Invaders emulation...\n";
    std::cout << "PC Inicial: " << std::hex << cpu.PC << "\n";

    for(long i = 0; i < 2000000; i++){
        cpu.cpuClock();

        if(i % 500000 == 0){
            std::cout << "Ciclos: " << i << "\n"; // debug
            cpu.dumpState();
        }
    }

    return 0;
}

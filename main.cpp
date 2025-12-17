#include <iostream>
#include "./memory/memory.hpp"
#include "./cpu/mos6502r.hpp"

int main() {
    Memory memory;
    Mos6502 cpu(&memory);

    memory.loadROM("./tests/6502_functional_test.bin");
    cpu.reset();

    for(int i = 0; i < 1000000; i++){
        cpu.cpuClock();

        if(i % 100000 == 0){
            cpu.dumpState();
        }
    }

    return 0;
}

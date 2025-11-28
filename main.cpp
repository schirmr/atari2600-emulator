#include <iostream>
#include <cassert>
#include "./memory/memory.hpp"
#include "./cpu/mos6502r.hpp"

int main() {
    Memory memory;
    Mos6502 cpu(&memory);

    memory.write(0x0000, 0xA9); // LDA imediato
    memory.write(0x0001, 0x42); // seta o valor y
    
    cpu.cpuClock();
    assert(cpu.A == 0x42);
    assert(cpu.getFlag(ZERO) == false); // teste flag ZERO
    assert(cpu.getFlag(NEGATIVE) == false); // teste flag NEGATIVE
    memory.dump(0x0000, 0x0010);
    //cpu.dumpState();
    /*while(true){
        cpu.cpuClock();
    }*/

    return 0;
}

#include <iostream>
#include "./memory/memory.hpp"
#include "./cpu/mos6502r.hpp"

int main() {
    Memory memory;
    Mos6502 cpu(&memory);

    memory.write(0x0000, 0xA9); // LDA imediato
    memory.write(0x0001, 0x42); // seta o valor y
    memory.write(0x0002, 0x00);

    memory.dump(0x0000, 0x0010);

    while(true){
        cpu.cpuClock();
    }

    return 0;
}

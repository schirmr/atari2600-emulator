#pragma once
#include <cstdint>
#include "../memory/memory.hpp"

class Mos6502 {
    public:
        uint8_t A = 0x00;
        uint8_t X = 0x00;
        uint8_t Y = 0x00;
        uint8_t SP = 0xFD; // Stack pointer
        uint16_t PC = 0x0000; // program counter
        uint8_t Status = 0x00; // flags NV-BDIZC (negative, overflow, -, break, decimal, interrupt disable, zero, carry)

        Memory* memory;
    public:
        Mos6502(Memory* mem);

        uint8_t busca();
        void cpuClock();
};
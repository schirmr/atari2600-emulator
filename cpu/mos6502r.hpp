#pragma once
#include <cstdint>
#include "../memory/memory.hpp"

enum FLAGS{
    CARRY = 1,  // 2⁰ = 1 (0x01)
    ZERO = 2, // 2¹ = 2 (0x02)
    INTERRUPT_DISABLE = 4, // 2² = 4 (0x04)
    DECIMAL_MODE = 8, // 2³ = 8 (0x08)
    BREAK = 16, // 2⁴ = 16 (0x10)
    UNUSED = 32, // 2⁵ = 32 (0x20)
    OVERFLOW = 64, // 2⁶ = 64 (0x40)
    NEGATIVE = 128 // 2⁷ = 128 (0x80)
};

class Mos6502 {
    public:
        uint8_t A = 0x00;
        uint8_t X = 0x00;
        uint8_t Y = 0x00;
        uint8_t SP = 0xFD; // Stack pointer
        uint16_t PC = 0x0000; // program counter
        uint8_t status = 0x20; // flags NV-BDIZC (negative, overflow, -, break, decimal, interrupt disable, zero, carry)

        Memory* memory;
    public:
        Mos6502(Memory* mem);

        uint8_t busca();

        void LDA(uint16_t address); // Load Accumulator

        uint16_t imm(); // modo imediato
        uint16_t zp(); // modo zero page
        uint16_t abs(); // modo absoluto
        uint16_t zpx(); // zero-page wrap
        uint16_t absx();
        uint16_t absy();
        uint16_t indx();
        uint16_t indy();

        void cpuClock();

        void setFlag(FLAGS flag, bool value);
        bool getFlag(FLAGS flag) const;
        void dumpState() const;
};
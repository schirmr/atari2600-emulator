#pragma once
#include <cstdint>

class Memory {
public:
    Memory();               // construtor
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);

    void dump(uint16_t start, uint16_t end) const; // 

private:
    uint8_t mem[65536];     // 64 KB
};

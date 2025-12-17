#pragma once
#include <cstdint>
#include <string>

class Memory {
public:
    Memory();               // construtor
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);

    void dump(uint16_t start, uint16_t end) const; // 

    void loadROM(const std::string& path); //
private:
    uint8_t mem[65536];     // 64 KB

    // ROM do cartucho
    uint8_t rom[8192];      // 8 KB
    uint16_t romSize = 0;

};

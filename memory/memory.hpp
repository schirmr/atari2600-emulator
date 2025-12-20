#pragma once
#include <cstdint>
#include <string>
#include "riot.hpp"

class Memory {
public:
    Memory();               // construtor
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);

    void dump(uint16_t start, uint16_t end) const; // 

    void loadROM(const std::string& path); //

    Riot riot;
private:
    uint8_t rom[4096];     // buffer para o cartucho de 4kb
    uint16_t romSize;
};

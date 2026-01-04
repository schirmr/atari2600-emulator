#pragma once
#include <cstdint>
#include <string>
#include "riot.hpp"
#include "../tia/tia.hpp"

class Memory {
public:
    Memory();               // construtor
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);
    void dump(uint16_t start, uint16_t end) const; // 
    bool loadROM(const std::string& path); //

    void step(uint32_t cycles){
        for(uint32_t i = 0; i < cycles; i++){
            riot.step(1);

            tia.clock();
            tia.clock();
            tia.clock();
        }
    }

    Riot riot;
    Tia tia;
private:
    enum class CartMapper : uint8_t {
        None, // ROM <= 4KB (ou espelhada)
        F8    // 8KB bankswitching (2x4KB) via hotspots $1FF8/$1FF9
    };

    uint8_t rom[8192];     // buffer para o cartucho (até 8KB neste projeto)
    uint16_t romSize;
    CartMapper mapper = CartMapper::None;
    mutable uint8_t activeBank = 0; // usado pelo mapper F8
};

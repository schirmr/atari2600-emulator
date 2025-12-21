#pragma once
#include <cstdint>

class Tia {
private:
    uint8_t registers[64]; // 64 registradores do TIA

public:
    Tia(); // Construtor
    
    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    
    uint8_t getReg(uint8_t index) const { return registers[index]; } // retorna no próprio hpp
};
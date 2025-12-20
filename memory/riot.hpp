#pragma once

#include <cstdint>
#include <iostream>

class Riot { // RIOT = (RAM, I/0, Timer) PIA6532
public:
    uint8_t ram[128]; // 128 bytes de RAM

private:
    // registradores de input e output
    uint8_t swcha;  // Port A Data (Joysticks)
    uint8_t swacnt; // Port A Data Direction Register (DDR)
    uint8_t swchb;  // Port B Data (Console switches: Reset, Select, P&B/Color)
    uint8_t swbcnt; // Port B Data Direction Register (DDR)

    // sistema de timer do PIA6532
    uint8_t  intim;          // O valor atual do timer (lido em $0284)
    uint16_t prescaler;      // O divisor atual (1, 8, 64 ou 1024)
    uint8_t  interruptFlag;  // Flag de interrupção (raro no Atari, mas existe no chip)
    
    uint32_t cyclesAccumulator; // Acumula ciclos da CPU até atingir o valor do prescaler

public:
    Riot();
    
    void reset();

    void step(uint32_t cycles);

    uint8_t ioRead(uint16_t addr);
    void ioWrite(uint16_t addr, uint8_t val);

    // metodos auxiliares para definir o estado dos botoes
    void setSWCHA(uint8_t val) { swcha = val; } // Joysticks
    void setSWCHB(uint8_t val) { swchb = val; } // Console (Reset, Select)
    uint8_t getSWCHA() const { return swcha; }
    uint8_t getSWCHB() const { return swchb; }
};
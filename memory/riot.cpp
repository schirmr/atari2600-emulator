#include "riot.hpp"

Riot::Riot() { // RIOT = (RAM, I/0, Timer) PIA6532  
    reset();
}

void Riot::reset() {
    for (int i = 0; i < 128; i++) ram[i] = 0;
    
    swcha = 0xFF; 
    swchb = 0xFF;  
    swacnt = 0x00;
    swbcnt = 0x00;

    intim = 0;
    prescaler = 1024;
    cyclesAccumulator = 0;
    interruptFlag = 0;
}

void Riot::step(uint32_t cycles) {
    cyclesAccumulator += cycles;

    while (cyclesAccumulator >= prescaler) {
        cyclesAccumulator -= prescaler;

        if (intim == 0) {
            interruptFlag = 1;
            
            // timer recomeça de 0xFF (255)
            intim = 0xFF;
            
            prescaler = 1; 
        } else {
            intim--;
        }
    }
}

uint8_t Riot::ioRead(uint16_t addr) {
    switch (addr & 0x07) {
        case 0x00: return swcha; // Leitura dos Joysticks
        case 0x02: return swchb; // Leitura do Console
        case 0x04: return intim; // Leitura do Timer
        case 0x05: // status de interrupção do Timer
            if(interruptFlag) {
                return 0x80; // retorna 10000000 (bit 7 setado se houve interrupção)
            } else return 0x00; // retorna 00000000 (sem interrupção)
        default: return 0x00;
    }
}

void Riot::ioWrite(uint16_t addr, uint8_t val) {
    if ((addr & 0x14) == 0x14) { 
        intim = val;
        cyclesAccumulator = 0; // Reseta contador de ciclos
        interruptFlag = 0;     // Limpa flag de interrupção
        
        switch (addr & 0x03) { // Verifica os 2 últimos bits para saber qual timer
            case 0: prescaler = 1; break;    // TIM1T
            case 1: prescaler = 8; break;    // TIM8T
            case 2: prescaler = 64; break;   // TIM64T
            case 3: prescaler = 1024; break; // TIM1024T
        }
        return;
    }

    // escrita de I/O (DDRs e Portas de saída)
    switch (addr & 0x03) {
        case 0x00: swcha = val; break;
        case 0x01: swacnt = val; break;
        case 0x02: swchb = val; break;
        case 0x03: swbcnt = val; break;
    }
}
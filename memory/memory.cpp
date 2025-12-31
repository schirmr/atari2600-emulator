#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "memory.hpp"
#include "riot.hpp"

Memory::Memory() {
    std::memset(rom, 0, sizeof(rom)); // evitar lixos
    romSize = 0;
    
    riot.reset();
}

uint8_t Memory::read(uint16_t addr) const {
    // 1. Checa a ROM
    if (addr & 0xF000) { // acesso a rom ($F000-$FFFF)
        return rom[addr & 0x0FFF]; // offset 0-4095
    }
    // 2. TIA read
    if ((addr & 0x0080) == 0) { // TIA read ($0000-$007F)
        uint8_t v = const_cast<Tia&>(tia).read(addr); // read do TIA pode limpar flags

        // Debug opcional: TRACE_INPUT=1 mostra mudancas no INPT4/INPT5.
        // TRACE_TIA=1 tambem mostra leituras de colisao (0x00..0x07).
        // Importante: usamos stderr + flush para aparecer no terminal.
        static bool trace = false;
        static bool traceInit = false;
        static bool traceTia = false;
        if (!traceInit) {
            const char* env = std::getenv("TRACE_INPUT");
            trace = (env && env[0] != '0');

            const char* env2 = std::getenv("TRACE_TIA");
            traceTia = (env2 && env2[0] != '0');

            if (trace || traceTia) {
                std::cerr << "[trace] TRACE_INPUT=" << (trace ? "1" : "0")
                          << " TRACE_TIA=" << (traceTia ? "1" : "0")
                          << std::endl;
            }
            traceInit = true;
        }

        if (trace || traceTia) {
            const uint8_t reg = static_cast<uint8_t>(addr & 0x3F);
            const uint8_t readIndex = reg & 0x0F;

            // INPT4/INPT5
            if (trace && (readIndex == 0x0C || readIndex == 0x0D)) {
                static uint8_t lastInpt4 = 0xFF;
                static uint8_t lastInpt5 = 0xFF;
                if (readIndex == 0x0C && v != lastInpt4) {
                    std::cerr << "INPT4 read = 0x" << std::hex << (int)v << std::dec << std::endl;
                    lastInpt4 = v;
                }
                if (readIndex == 0x0D && v != lastInpt5) {
                    std::cerr << "INPT5 read = 0x" << std::hex << (int)v << std::dec << std::endl;
                    lastInpt5 = v;
                }
            }

            // Colisoes 0x00..0x07
            if (traceTia && readIndex <= 0x07) {
                static uint8_t lastCx[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
                const int idx = (int)readIndex;
                if (v != lastCx[idx]) {
                    lastCx[idx] = v;
                }
            }
        }

        return v;
    }
    // 3. Ram e stack
    if ((addr & 0x0280) == 0x0080) {  // Acesso à RAM e Stack ($0080-$00FF e $0180-$01FF)
        return riot.ram[addr & 0x007F]; // map para 0-127 dentro do array da RAM
    }
    // 4. Riot I/O e Timer
    if ((addr & 0x0280) == 0x0280) {  // leitura registradores PIA (Timer/Ports) - $0280-$0297
        return const_cast<Riot&>(riot).ioRead(addr); 
    }

    // acesso ao TIA
    // por enquanto retorna 0, ler colisão depois
    return 0x00;
}

void Memory::write(uint16_t addr, uint8_t data) {
    if((addr & 0x0080) == 0) { // Escrita no TIA ($0000-$007F)
        // Trace opcional de writes no TIA, para depurar tiros (ENAMx/RESMx/GRPx)
        static bool traceTiaW = false;
        static bool traceInitW = false;
        if (!traceInitW) {
            const char* env = std::getenv("TRACE_TIA");
            traceTiaW = (env && env[0] != '0');
            traceInitW = true;
        }

        if (traceTiaW) {
            const uint8_t reg = static_cast<uint8_t>(addr & 0x3F);
            // Loga apenas registradores relevantes para tiros/colisoes para nao virar spam.
            const bool interesting = (reg == 0x10) || (reg == 0x11) || (reg == 0x12) || (reg == 0x13) ||
                                     (reg == 0x14) || (reg == 0x1B) || (reg == 0x1C) || (reg == 0x1D) ||
                                     (reg == 0x1E) || (reg == 0x1F) || (reg == 0x28) || (reg == 0x29) ||
                                     (reg == 0x2C);
            if (interesting) {
                static uint8_t lastWrite[64] = {0};
                static bool lastWriteInit = false;
                if (!lastWriteInit) {
                    for (int i = 0; i < 64; ++i) lastWrite[i] = 0xFF;
                    lastWriteInit = true;
                }
                if (data != lastWrite[reg]) {
                    lastWrite[reg] = data;
                }
            }
        }

        tia.write(addr, data);

        if (tia.isWSYNCActive()) {
            while (tia.isWSYNCActive()) {
                // TIA roda 3 clocks por ciclo da CPU
                tia.clock();
                tia.clock();
                tia.clock();
                riot.step(1);
            }
        }
        return;
    }

    if ((addr & 0x0280) == 0x0080) {     // Escrita na RAM e Stack ($0080-$00FF e $0180-$01FF)
        riot.ram[addr & 0x007F] = data;
        return;
    }

    if ((addr & 0x0280) == 0x0280) {  // escrita registradores PIA (Timer/Ports) - $0280-$0297
        riot.ioWrite(addr, data);
        return;
    }

    // desenho da tela
    if ((addr & 0xF000) == 0) { // Escrita no TIA ($0000-$007F)
        // logica de escrita do TIA
        return;
    }
}

void Memory::dump(uint16_t start, uint16_t end) const{
    for(uint16_t addr = start; addr <= end; addr++){
        printf("%04X: %02X\n", addr, read(addr));
    }
}

bool Memory::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erro ao abrir ROM\n";
        return false;
    }

    file.read((char*)rom, sizeof(rom));
    romSize = (uint16_t)file.gcount();

    std::cout << "ROM carregada: " << romSize << " bytes\n";
    return true;
}

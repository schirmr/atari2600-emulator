#include "tia.hpp"
#include <iostream>

Tia::Tia() {
    reset();
}

void Tia::clock() {
    tiaCycle++;
    vsyncActive = (registers[0x00] & 0x02) != 0; // VSYNC bit (D1)
    vblankActive = (registers[0x01] & 0x02) != 0; // VBLANK bit (D1)

    if (!vblankActive && !vsyncActive) {
        if (tiaCycle < VISIBLE_CYCLES) {
            auto pfBitAt = [&](int x) -> bool {
                int block = x / 4; // cada bit do playfield dura 4 clocks -> 160 px
                bool reflect = (registers[0x0A] & 0x01) != 0; // CTRLPF bit0 = REFLECT

                auto leftBit = [&](int idx) -> bool {
                    if (idx < 4) { // PF0 (bits D4..D7) mapeados da esquerda para direita
                        uint8_t pf0 = registers[0x0D];
                        return ((pf0 >> (4 + idx)) & 0x01) != 0;
                    } else if (idx < 12) { // PF1 desenhado MSB->LSB
                        uint8_t pf1 = registers[0x0E];
                        int bit = 7 - (idx - 4);
                        return ((pf1 >> bit) & 0x01) != 0;
                    } else { // PF2 desenhado LSB->MSB
                        uint8_t pf2 = registers[0x0F];
                        int bit = (idx - 12);
                        return ((pf2 >> bit) & 0x01) != 0;
                    }
                };

                if (block < 20) {
                    return leftBit(block);
                } else {
                    int r = block - 20;
                    if (reflect) {
                        return leftBit(19 - r);
                    } else {
                        // sem reflexao: repete mesma ordem da metade esquerda
                        return leftBit(r);
                    }
                }
            };

            bool pf = pfBitAt(tiaCycle);
            uint8_t colupf = registers[0x08];
            uint8_t colubk = registers[0x09];
            uint8_t color = pf ? colupf : colubk;
            framebuffer[scanline][tiaCycle] = color;
        }
    }
    if (tiaCycle >= SCANLINE_CYCLES) {
        tiaCycle = 0;
        scanline++;
        if (scanline >= FRAME_LINES) {
            scanline = 0;
        }
        // contabiliza linhas enquanto VSYNC esta ativo
        if (vsyncActive) {
            vsyncLines++;
        }
        if (!vsyncActive && vsyncPrevActive) {
            if (vsyncLines >= 3) {
                scanline = 0;
            }
            vsyncLines = 0;
        }
        vsyncPrevActive = vsyncActive;
        if (wsync) {
            wsync = false; // termina a espera do WSYNC no fim do scanline
        }
    }
}

bool Tia::endOfScanline() const {
    return tiaCycle == 227;
}

void Tia::reset() {
    for(int i=0; i<64; i++) registers[i] = 0;
    vsyncActive = false;
    vblankActive = false;
    vsyncLines = 0;
    vsyncPrevActive = false;
    // limpa framebuffer
    for (int y = 0; y < FRAME_LINES; ++y) {
        for (int x = 0; x < VISIBLE_CYCLES; ++x) {
            framebuffer[y][x] = 0;
        }
    }
}

uint8_t Tia::read(uint16_t addr) {
    uint8_t reg = addr & 0x3F;

    if (reg >= 0x30 && reg <= 0x3D) {
        // retornar colisao e inputs futuramente
        return 0x00; 
    }

    return registers[reg]; 
}

void Tia::write(uint16_t addr, uint8_t val) {
    uint8_t reg = addr & 0x3F;
    
    registers[reg] = val;

    if(debug && reg == 0x09) std::cout << "Cor de fundo: " << std::hex << (int)val << "\n"; // debug das cores mudando

    if(reg == 0x02) {
        wsync = true; 
    }
    // VSYNC/VBLANK seguem bits especificos: D1
    if (reg == 0x00) {
        vsyncActive = (val & 0x02) != 0;
        if (vsyncActive && !vsyncPrevActive) {
            vsyncLines = 0; // reinicia contagem ao ligar VSYNC
        }
    }
    if (reg == 0x01) {
        vblankActive = (val & 0x02) != 0;
    }
}
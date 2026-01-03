#include "tia.hpp"
#include <iostream>

static constexpr uint8_t TIA_VSYNC  = 0x00;
static constexpr uint8_t TIA_VBLANK = 0x01;
static constexpr uint8_t TIA_WSYNC  = 0x02;
static constexpr uint8_t TIA_NUSIZ0 = 0x04;
static constexpr uint8_t TIA_NUSIZ1 = 0x05;
static constexpr uint8_t TIA_COLUP0 = 0x06;
static constexpr uint8_t TIA_COLUP1 = 0x07;
static constexpr uint8_t TIA_COLUPF = 0x08;
static constexpr uint8_t TIA_COLUBK = 0x09;
static constexpr uint8_t TIA_CTRLPF = 0x0A;
static constexpr uint8_t TIA_REFP0  = 0x0B;
static constexpr uint8_t TIA_REFP1  = 0x0C;
static constexpr uint8_t TIA_PF0    = 0x0D;
static constexpr uint8_t TIA_PF1    = 0x0E;
static constexpr uint8_t TIA_PF2    = 0x0F;
static constexpr uint8_t TIA_RESP0  = 0x10;
static constexpr uint8_t TIA_RESP1  = 0x11;
static constexpr uint8_t TIA_RESM0  = 0x12;
static constexpr uint8_t TIA_RESM1  = 0x13;
static constexpr uint8_t TIA_RESBL  = 0x14;
static constexpr uint8_t TIA_GRP0   = 0x1B;
static constexpr uint8_t TIA_GRP1   = 0x1C;
static constexpr uint8_t TIA_ENAM0  = 0x1D;
static constexpr uint8_t TIA_ENAM1  = 0x1E;
static constexpr uint8_t TIA_ENABL  = 0x1F;
static constexpr uint8_t TIA_HMP0   = 0x20;
static constexpr uint8_t TIA_HMP1   = 0x21;
static constexpr uint8_t TIA_HMM0   = 0x22;
static constexpr uint8_t TIA_HMM1   = 0x23;
static constexpr uint8_t TIA_HMBL   = 0x24;
static constexpr uint8_t TIA_RESMP0 = 0x28;
static constexpr uint8_t TIA_RESMP1 = 0x29;
static constexpr uint8_t TIA_HMOVE  = 0x2A;
static constexpr uint8_t TIA_HMCLR  = 0x2B;
static constexpr uint8_t TIA_CXCLR  = 0x2C;

static constexpr uint8_t TIA_CXM0P  = 0x30;
static constexpr uint8_t TIA_CXM1P  = 0x31;
static constexpr uint8_t TIA_CXP0FB = 0x32;
static constexpr uint8_t TIA_CXP1FB = 0x33;
static constexpr uint8_t TIA_CXM0FB = 0x34;
static constexpr uint8_t TIA_CXM1FB = 0x35;
static constexpr uint8_t TIA_CXBLPF = 0x36;
static constexpr uint8_t TIA_CXPPMM = 0x37;

Tia::Tia() {
    reset();
}

int Tia::strobeToVisibleX() const {
    static constexpr int STROBE_PIPELINE_DELAY = 0;
    static constexpr int STROBE_X_OFFSET = 8; // 1 bloco de playfield (cada bit PF = 4px)
    int x = tiaCycle - HBLANK_CYCLES - STROBE_PIPELINE_DELAY + STROBE_X_OFFSET;
    x %= VISIBLE_CYCLES;
    if (x < 0) x += VISIBLE_CYCLES;
    return x;
}

int Tia::motionFromHMM(uint8_t v) {
    // Movimento fino usa o nibble alto (bits 7..4) em complemento de dois (4-bit): -8..+7
    int n = static_cast<int>((v >> 4) & 0x0F);
    if (n >= 8) n -= 16;
    // Direção do HMOVE: para este core simplificado, o sinal invertido
    // costuma alinhar melhor com kernels reais (reduz drift/saltos).
    return -n;
}

int Tia::missileWidthFromNUSIZ(uint8_t nusiz) {
    // TIA: largura do míssil em bits 5..4 do NUSIZx
    switch ((nusiz >> 4) & 0x03) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        default: return 1;
    }
}

int Tia::ballWidthFromCTRLPF(uint8_t ctrlpf) {
    // CTRLPF bits 5..4 controlam tamanho da BALL: 1,2,4,8
    switch ((ctrlpf >> 4) & 0x03) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        default: return 1;
    }
}

void Tia::decodeNUSIZPlayer(uint8_t nusiz, int& scaleOut, int offsetsOut[3], int& countOut) {
    // Suporte ao subset mais usado:
    // - escala 1x/2x/4x
    // - 1/2/3 cópias com espaçamentos aproximados
    // Nota: o TIA real usa padrões específicos; aqui buscamos compatibilidade prática.
    const int mode = nusiz & 0x07;

    scaleOut = 1;
    countOut = 1;
    offsetsOut[0] = 0;
    offsetsOut[1] = 0;
    offsetsOut[2] = 0;

    // Espaçamentos típicos em color clocks (aproximação): 16/32/64
    auto setCopies = [&](int count, int o1, int o2) {
        countOut = count;
        offsetsOut[0] = 0;
        offsetsOut[1] = o1;
        offsetsOut[2] = o2;
    };

    switch (mode) {
        case 0: // 1 cópia
            setCopies(1, 0, 0);
            break;
        case 1: // 2 cópias “close”
            setCopies(2, 16, 0);
            break;
        case 2: // 2 cópias “medium”
            setCopies(2, 32, 0);
            break;
        case 3: // 3 cópias “close”
            setCopies(3, 16, 32);
            break;
        case 4: // 2 cópias “wide”
            setCopies(2, 64, 0);
            break;
        case 5: // 1 cópia “double”
            setCopies(1, 0, 0);
            scaleOut = 2;
            break;
        case 6: // 3 cópias “medium”
            setCopies(3, 32, 64);
            break;
        case 7: // 1 cópia “quadruple”
            setCopies(1, 0, 0);
            scaleOut = 4;
            break;
        default:
            setCopies(1, 0, 0);
            break;
    }
}

bool Tia::playfieldPixelOn(int x) const {
    int block = x / 4; // cada bit do playfield dura 4 clocks -> 160 px
    bool reflect = (registers[TIA_CTRLPF] & 0x01) != 0; // CTRLPF bit0 = REFLECT

    auto leftBit = [&](int idx) -> bool {
        if (idx < 4) { // PF0 (bits D4..D7) mapeados da esquerda para direita
            uint8_t pf0 = registers[TIA_PF0];
            return ((pf0 >> (4 + idx)) & 0x01) != 0;
        } else if (idx < 12) { // PF1 desenhado MSB->LSB
            uint8_t pf1 = registers[TIA_PF1];
            int bit = 7 - (idx - 4);
            return ((pf1 >> bit) & 0x01) != 0;
        } else { // PF2 desenhado LSB->MSB
            uint8_t pf2 = registers[TIA_PF2];
            int bit = (idx - 12);
            return ((pf2 >> bit) & 0x01) != 0;
        }
    };

    if (block < 20) {
        return leftBit(block);
    }

    int r = block - 20;
    if (reflect) {
        return leftBit(19 - r);
    }
    // sem reflexao: repete mesma ordem da metade esquerda
    return leftBit(r);
}

uint8_t Tia::playfieldColorForX(int x) const {
    const uint8_t ctrlpf = registers[TIA_CTRLPF];
    const bool scoreboard = (ctrlpf & 0x02) != 0; // CTRLPF bit1
    if (!scoreboard) {
        return registers[TIA_COLUPF];
    }
    // modo "scoreboard": metade esquerda usa COLUP0, direita usa COLUP1
    return (x < (VISIBLE_CYCLES / 2)) ? registers[TIA_COLUP0] : registers[TIA_COLUP1];
}

bool Tia::playerPixelOn(int x, int playerIndex) const {
    const int baseX = (playerIndex == 0) ? p0X : p1X;
    const uint8_t nusiz = registers[(playerIndex == 0) ? TIA_NUSIZ0 : TIA_NUSIZ1];
    const uint8_t grp = registers[(playerIndex == 0) ? TIA_GRP0 : TIA_GRP1];
    const bool reflect = (registers[(playerIndex == 0) ? TIA_REFP0 : TIA_REFP1] & 0x08) != 0;

    int scale = 1;
    int offsets[3] = {0, 0, 0};
    int count = 1;
    decodeNUSIZPlayer(nusiz, scale, offsets, count);

    for (int i = 0; i < count; ++i) {
        const int start = baseX + offsets[i];

        // Região potencial do sprite (8 * scale)
        // Sem wrap intrínseco no sprite; se ultrapassar 159, simplesmente "corta".
        const int dx = x - start;
        if (dx < 0) continue;
        const int bitIndex = dx / scale;
        if (bitIndex < 0 || bitIndex > 7) continue;

        int srcBit = reflect ? bitIndex : (7 - bitIndex);
        if (((grp >> srcBit) & 0x01) != 0) {
            return true;
        }
    }
    return false;
}

bool Tia::missilePixelOn(int x, int missileIndex) const {
    const bool enabled = (missileIndex == 0) ? m0Enabled : m1Enabled;
    if (!enabled) return false;

    const int baseX = (missileIndex == 0) ? m0X : m1X;
    const uint8_t nusiz = registers[(missileIndex == 0) ? TIA_NUSIZ0 : TIA_NUSIZ1];
    const int width = missileWidthFromNUSIZ(nusiz);

    // Também respeita múltiplas cópias do NUSIZ (mesmo padrão do player)
    int scaleIgnored = 1;
    int offsets[3] = {0, 0, 0};
    int count = 1;
    decodeNUSIZPlayer(nusiz, scaleIgnored, offsets, count);

    for (int i = 0; i < count; ++i) {
        const int start = baseX + offsets[i];
        const int dx = x - start;
        if (dx >= 0 && dx < width) {
            return true;
        }
    }
    return false;
}

bool Tia::ballPixelOn(int x) const {
    if (!blEnabled) return false;
    const int width = ballWidthFromCTRLPF(registers[TIA_CTRLPF]);
    const int dx = x - blX;
    return (dx >= 0 && dx < width);
}

void Tia::latchCollisions(bool pfOn, bool p0On, bool p1On, bool m0On, bool m1On, bool blOn) {
    // Colisões são latched até CXCLR.
    // Bits usados são D7 e D6.
    if (m0On && p1On) registers[TIA_CXM0P]  |= 0x80;
    if (m0On && p0On) registers[TIA_CXM0P]  |= 0x40;

    if (m1On && p0On) registers[TIA_CXM1P]  |= 0x80;
    if (m1On && p1On) registers[TIA_CXM1P]  |= 0x40;

    if (p0On && pfOn) registers[TIA_CXP0FB] |= 0x80;
    if (p0On && blOn) registers[TIA_CXP0FB] |= 0x40;

    if (p1On && pfOn) registers[TIA_CXP1FB] |= 0x80;
    if (p1On && blOn) registers[TIA_CXP1FB] |= 0x40;

    if (m0On && pfOn) registers[TIA_CXM0FB] |= 0x80;
    if (m0On && blOn) registers[TIA_CXM0FB] |= 0x40;

    if (m1On && pfOn) registers[TIA_CXM1FB] |= 0x80;
    if (m1On && blOn) registers[TIA_CXM1FB] |= 0x40;

    if (blOn && pfOn) registers[TIA_CXBLPF] |= 0x80;

    if (p0On && p1On) registers[TIA_CXPPMM] |= 0x80;
    if (m0On && m1On) registers[TIA_CXPPMM] |= 0x40;
}

void Tia::clock() {
    // beam = posição atual dentro da scanline (0..227)
    const int beam = tiaCycle;

    // HMOVE (simplificado): aplica no começo da scanline
    if (beam == 0 && hmovePending) {
        p0X += pendingP0;
        p1X += pendingP1;

        // Mísseis: se presos ao player, acompanham
        if ((registers[TIA_RESMP0] & 0x02) != 0) {
            m0X = p0X;
        } else {
            m0X += pendingM0;
        }

        if ((registers[TIA_RESMP1] & 0x02) != 0) {
            m1X = p1X;
        } else {
            m1X += pendingM1;
        }

        blX += pendingBL;

        hmovePending = false;
        pendingP0 = pendingP1 = pendingM0 = pendingM1 = pendingBL = 0;
    }

    // Atualiza sinais
    vsyncActive = (registers[TIA_VSYNC] & 0x02) != 0;  // VSYNC bit (D1)
    vblankActive = (registers[TIA_VBLANK] & 0x02) != 0; // VBLANK bit (D1)

    // Cache enable bits
    m0Enabled = (registers[TIA_ENAM0] & 0x02) != 0;
    m1Enabled = (registers[TIA_ENAM1] & 0x02) != 0;
    blEnabled = (registers[TIA_ENABL] & 0x02) != 0;

    // Renderização visível: janela de 160px começa após HBLANK
    const int x = beam - HBLANK_CYCLES;
    if (x >= 0 && x < VISIBLE_CYCLES) {
        // Durante VSYNC/VBLANK, o vídeo fica em preto. Importante para não
        // deixar "lixo" de frames anteriores (flicker no rodapé/overscan).
        if (vsyncActive || vblankActive) {
            framebuffer[scanline][x] = 0;
        } else {
            const bool pfOn = playfieldPixelOn(x);
            const bool blOn = ballPixelOn(x);
            const bool p0On = playerPixelOn(x, 0);
            const bool p1On = playerPixelOn(x, 1);
            const bool m0On = missilePixelOn(x, 0);
            const bool m1On = missilePixelOn(x, 1);

            latchCollisions(pfOn, p0On, p1On, m0On, m1On, blOn);

            const uint8_t bg = registers[TIA_COLUBK];
            const uint8_t pfCol = playfieldColorForX(x);
            const uint8_t blCol = registers[TIA_COLUPF];
            const uint8_t p0Col = registers[TIA_COLUP0];
            const uint8_t p1Col = registers[TIA_COLUP1];

            // Prioridade: CTRLPF bit2
            const bool pfPriority = (registers[TIA_CTRLPF] & 0x04) != 0;

            uint8_t out = bg;

            auto drawPlayers = [&]() {
                if (p0On) return p0Col;
                if (m0On) return p0Col;
                if (p1On) return p1Col;
                if (m1On) return p1Col;
                return bg;
            };
            auto drawPF = [&]() {
                if (blOn) return blCol;
                if (pfOn) return pfCol;
                return bg;
            };

            if (pfPriority) {
                // PF/Ball na frente
                out = drawPlayers();
                uint8_t pfOut = drawPF();
                if (pfOut != bg) out = pfOut;
            } else {
                // Players/Missiles na frente
                out = drawPF();
                uint8_t plOut = drawPlayers();
                if (plOut != bg) out = plOut;
            }

            framebuffer[scanline][x] = out;
        }
    }

    // Avança o feixe
    tiaCycle++;
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
    return tiaCycle == (SCANLINE_CYCLES - 1);
}

void Tia::reset() {
    for(int i=0; i<64; i++) registers[i] = 0;
    vsyncActive = false;
    vblankActive = false;
    vsyncLines = 0;
    vsyncPrevActive = false;

    // estado de objetos
    p0X = p1X = m0X = m1X = blX = 0;
    m0Enabled = m1Enabled = blEnabled = false;

    hmovePending = false;
    pendingP0 = pendingP1 = pendingM0 = pendingM1 = pendingBL = 0;

    trigger0Pressed = false;
    trigger1Pressed = false;

    inputLatchEnabled = false;
    latchedTrigger0Pressed = false;
    latchedTrigger1Pressed = false;

    // colisões (latches)
    registers[TIA_CXM0P] = 0;
    registers[TIA_CXM1P] = 0;
    registers[TIA_CXP0FB] = 0;
    registers[TIA_CXP1FB] = 0;
    registers[TIA_CXM0FB] = 0;
    registers[TIA_CXM1FB] = 0;
    registers[TIA_CXBLPF] = 0;
    registers[TIA_CXPPMM] = 0;

    // limpa framebuffer
    for (int y = 0; y < FRAME_LINES; ++y) {
        for (int x = 0; x < VISIBLE_CYCLES; ++x) {
            framebuffer[y][x] = 0;
        }
    }
}

uint8_t Tia::read(uint16_t addr) {
    uint8_t reg = addr & 0x3F;

    // Leituras do TIA so espelhadas a cada 0x10.
    // Ex.: INPT4 pode ser lido em 0x0C, 0x1C, 0x2C ou 0x3C.
    const uint8_t readIndex = reg & 0x0F;

    // Colises (latched): leitura em 0x00..0x07 (com espelhos)
    if (readIndex <= 0x07) {
        return registers[0x30u + readIndex];
    }

    // Inputs (INPTx): leitura em 0x08..0x0D (com espelhos)
    // INPT4 (0x0C) / INPT5 (0x0D) = trigger do joystick.
    // Bit7  o relevante: 1 solto, 0 pressionado (active low).
    if (readIndex == 0x0C) {
        const bool pressed = inputLatchEnabled ? latchedTrigger0Pressed : trigger0Pressed;
        return pressed ? 0x00 : 0x80;
    }
    if (readIndex == 0x0D) {
        const bool pressed = inputLatchEnabled ? latchedTrigger1Pressed : trigger1Pressed;
        return pressed ? 0x00 : 0x80;
    }

    // INPT0..INPT3 (paddles): por enquanto sempre "solto"
    if (readIndex >= 0x08 && readIndex <= 0x0B) {
        return 0x80;
    }

    return registers[reg];
}

void Tia::write(uint16_t addr, uint8_t val) {
    uint8_t reg = addr & 0x3F;

    // Registradores de colisão/inputs são read-only do ponto de vista do jogo.
    if (reg >= 0x30 && reg <= 0x3F) {
        // Ignora writes diretos; limpeza acontece via CXCLR.
        return;
    }

    registers[reg] = val;

    if(debug && reg == 0x09) std::cout << "Cor de fundo: " << std::hex << (int)val << "\n"; // debug das cores mudando

    if(reg == TIA_WSYNC) {
        wsync = true; 
    }
    // VSYNC/VBLANK seguem bits especificos: D1
    if (reg == TIA_VSYNC) {
        vsyncActive = (val & 0x02) != 0;
        if (vsyncActive && !vsyncPrevActive) {
            vsyncLines = 0; // reinicia contagem ao ligar VSYNC
        }
    }
    if (reg == TIA_VBLANK) {
        vblankActive = (val & 0x02) != 0;

        // VBLANK bit6 controla latch de inputs.
        // Implementação prática:
        // - Ao habilitar, captura o estado atual.
        // - Enquanto habilitado, setTriggerXPressed(true) faz o latch "grudar".
        // - Ao desabilitar, limpa o latch.
        const bool newLatchEnabled = (val & 0x40) != 0;
        if (newLatchEnabled != inputLatchEnabled) {
            inputLatchEnabled = newLatchEnabled;
            if (inputLatchEnabled) {
                latchedTrigger0Pressed = trigger0Pressed;
                latchedTrigger1Pressed = trigger1Pressed;
            } else {
                latchedTrigger0Pressed = false;
                latchedTrigger1Pressed = false;
            }
        }
    }

    // ------------------------------
    // Strobed registers / TIA avançado
    // ------------------------------
    if (reg == TIA_RESP0) {
        p0X = strobeToVisibleX();
        // Se RESMP0 está ativo, prende o míssil em P0.
        if ((registers[TIA_RESMP0] & 0x02) != 0) {
            m0X = p0X;
        }
        return;
    }
    if (reg == TIA_RESP1) {
        p1X = strobeToVisibleX();
        if ((registers[TIA_RESMP1] & 0x02) != 0) {
            m1X = p1X;
        }
        return;
    }
    if (reg == TIA_RESM0) {
        m0X = strobeToVisibleX();
        return;
    }
    if (reg == TIA_RESM1) {
        m1X = strobeToVisibleX();
        return;
    }
    if (reg == TIA_RESBL) {
        blX = strobeToVisibleX();
        return;
    }

    if (reg == TIA_RESMP0) {
        // Bit D1: reset missile to player
        if ((val & 0x02) != 0) {
            m0X = p0X;
        }
        return;
    }
    if (reg == TIA_RESMP1) {
        if ((val & 0x02) != 0) {
            m1X = p1X;
        }
        return;
    }

    if (reg == TIA_HMOVE) {
        // HMOVE é strobed; aplicamos no começo da scanline seguinte.
        pendingP0 = motionFromHMM(registers[TIA_HMP0]);
        pendingP1 = motionFromHMM(registers[TIA_HMP1]);
        pendingM0 = motionFromHMM(registers[TIA_HMM0]);
        pendingM1 = motionFromHMM(registers[TIA_HMM1]);
        pendingBL = motionFromHMM(registers[TIA_HMBL]);
        hmovePending = true;
        return;
    }

    if (reg == TIA_HMCLR) {
        registers[TIA_HMP0] = 0;
        registers[TIA_HMP1] = 0;
        registers[TIA_HMM0] = 0;
        registers[TIA_HMM1] = 0;
        registers[TIA_HMBL] = 0;
        return;
    }

    if (reg == TIA_CXCLR) {
        registers[TIA_CXM0P] = 0;
        registers[TIA_CXM1P] = 0;
        registers[TIA_CXP0FB] = 0;
        registers[TIA_CXP1FB] = 0;
        registers[TIA_CXM0FB] = 0;
        registers[TIA_CXM1FB] = 0;
        registers[TIA_CXBLPF] = 0;
        registers[TIA_CXPPMM] = 0;
        return;
    }
}
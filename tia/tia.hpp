#pragma once
#include <cstdint>

class Tia {
private:
    uint8_t registers[64]; // 64 registradores do TIA

    static constexpr int SCANLINE_CYCLES = 228; // valores fixos
    static constexpr int VISIBLE_CYCLES = 160; // região visível simplificada
    static constexpr int FRAME_LINES = 262; // valores fixos

    // 228 color clocks por scanline; parte visível normalmente começa após o HBLANK.
    static constexpr int HBLANK_CYCLES = 68;

    int tiaCycle = 0; // 0-227 (228 clocks por scanline)
    int scanline = 0; // 0-261 (262 scanlines por frame)

    bool wsync = false; // flag para esperar o fim do scanline
    bool vsyncActive = false; // VSYNC ativo por ~3 linhas
    bool vblankActive = false; // VBLANK ativo durante parte do frame
    int vsyncLines = 0; // contador de linhas em VSYNC
    bool vsyncPrevActive = false; // estado anterior do VSYNC

    // Framebuffer básico por scanline (cor de fundo apenas)
    uint8_t framebuffer[FRAME_LINES][VISIBLE_CYCLES]{};

    // ------------------------------
    // Estado de objetos (TIA avançado - Etapa 7)
    // ------------------------------
    // Posicionamento horizontal em pixels (0..159) na região visível.
    int p0X = 0;
    int p1X = 0;
    int m0X = 0;
    int m1X = 0;
    int blX = 0;

    // Cache do enable de mísseis/ball (bit D1 dos registradores ENAMx/ENABL)
    bool m0Enabled = false;
    bool m1Enabled = false;
    bool blEnabled = false;

    // HMOVE: aplicado no começo da scanline seguinte (simplificação compatível)
    bool hmovePending = false;
    int pendingP0 = 0;
    int pendingP1 = 0;
    int pendingM0 = 0;
    int pendingM1 = 0;
    int pendingBL = 0;

    // Helpers
    int strobeToVisibleX() const;
    static int motionFromHMM(uint8_t v);
    static int missileWidthFromNUSIZ(uint8_t nusiz);
    static int ballWidthFromCTRLPF(uint8_t ctrlpf);
    static void decodeNUSIZPlayer(uint8_t nusiz, int& scaleOut, int offsetsOut[3], int& countOut);
    bool playerPixelOn(int x, int playerIndex) const;
    bool missilePixelOn(int x, int missileIndex) const;
    bool ballPixelOn(int x) const;
    bool playfieldPixelOn(int x) const;
    uint8_t playfieldColorForX(int x) const;
    void latchCollisions(bool pfOn, bool p0On, bool p1On, bool m0On, bool m1On, bool blOn);

    bool debug = false; // controla logs de debug

    // Inputs (simplificado): trigger do joystick
    bool trigger0Pressed = false; // INPT4
    bool trigger1Pressed = false; // INPT5

    // Latch de inputs controlado por VBLANK bit 6.
    // Quando ativo, o trigger pode "grudar" at que o latch seja limpo.
    bool inputLatchEnabled = false;
    bool latchedTrigger0Pressed = false;
    bool latchedTrigger1Pressed = false;

public:
    Tia(); // Construtor
    
    void reset();

    void clock();
    bool endOfScanline() const;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void setDebug(bool enabled) { debug = enabled; }
    bool isDebug() const { return debug; }

    // Inputs (disparo) - active low no hardware real; aqui armazenamos como bool "pressed"
    void setTrigger0Pressed(bool pressed) {
        trigger0Pressed = pressed;
        if (inputLatchEnabled && pressed) {
            latchedTrigger0Pressed = true;
        }
    }

    void setTrigger1Pressed(bool pressed) {
        trigger1Pressed = pressed;
        if (inputLatchEnabled && pressed) {
            latchedTrigger1Pressed = true;
        }
    }
    
    bool isWSYNCActive() const { return wsync; }
    bool inVBlank() const { return vblankActive; }
    bool inVSync() const { return vsyncActive; }
    int getScanline() const { return scanline; }
    int getCycle() const { return tiaCycle; }
    const uint8_t* getScanlineBuffer(int y) const { return (y >= 0 && y < FRAME_LINES) ? framebuffer[y] : nullptr; }
    const uint8_t* getFrameBuffer() const { return &framebuffer[0][0]; }
    
    uint8_t getReg(uint8_t index) const { return registers[index]; } // retorna no próprio hpp
};
#pragma once
#include <cstdint>

class Tia {
private:
    uint8_t registers[64]; // 64 registradores do TIA

    static constexpr int SCANLINE_CYCLES = 228; // valores fixos
    static constexpr int VISIBLE_CYCLES = 160; // região visível simplificada
    static constexpr int FRAME_LINES = 262; // valores fixos

    int tiaCycle = 0; // 0-227 (228 clocks por scanline)
    int scanline = 0; // 0-261 (262 scanlines por frame)

    bool wsync = false; // flag para esperar o fim do scanline
    bool vsyncActive = false; // VSYNC ativo por ~3 linhas
    bool vblankActive = false; // VBLANK ativo durante parte do frame
    int vsyncLines = 0; // contador de linhas em VSYNC
    bool vsyncPrevActive = false; // estado anterior do VSYNC

    // Framebuffer básico por scanline (cor de fundo apenas)
    uint8_t framebuffer[FRAME_LINES][VISIBLE_CYCLES]{};

    bool debug = false; // controla logs de debug

public:
    Tia(); // Construtor
    
    void reset();

    void clock();
    bool endOfScanline() const;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void setDebug(bool enabled) { debug = enabled; }
    bool isDebug() const { return debug; }
    
    bool isWSYNCActive() const { return wsync; }
    bool inVBlank() const { return vblankActive; }
    bool inVSync() const { return vsyncActive; }
    int getScanline() const { return scanline; }
    int getCycle() const { return tiaCycle; }
    const uint8_t* getScanlineBuffer(int y) const { return (y >= 0 && y < FRAME_LINES) ? framebuffer[y] : nullptr; }
    const uint8_t* getFrameBuffer() const { return &framebuffer[0][0]; }
    
    uint8_t getReg(uint8_t index) const { return registers[index]; } // retorna no próprio hpp
};
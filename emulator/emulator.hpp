#pragma once
#include <string>
#include "../memory/memory.hpp"
#include "../cpu/mos6502r.hpp"

#ifdef _WIN32
#include "../graphics/win32_renderer.hpp"
#else
#include "../graphics/sdl2_renderer.hpp"
#endif

class Emulator {
public:
    // A classe Emulator é um "orquestrador":
    // - carrega ROM
    // - reseta CPU
    // - executa o loop principal
    // - chama o renderer para mostrar um frame
    //
    // Importante: aqui a gente não está tentando ser 100% fiel ainda.
    // O foco é didático e incremental.
    Emulator();
    bool loadROM(const std::string& path);
    void run();

private:
    // Executa "um passo" de emulação:
    // - roda CPU (1 instrução / clock)
    // - sincroniza TIA/RIOT com a proporção correta
    void step();

    // Detecta quando completamos um frame.
    // A heurística atual é: scanline foi de 261 -> 0.
    bool endOfFrame();

    Memory memory;
    Mos6502 cpu;

#ifdef _WIN32
    Win32Renderer renderer;
#else
    Sdl2Renderer renderer;
#endif

    // Guarda o scanline do ciclo anterior para detectar "virada" de frame.
    int lastScanline = 0;
};

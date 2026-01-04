#include "emulator.hpp"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

// Construtor:
// - Conecta a CPU no barramento (Memory)
// - O renderer é inicializado no run() (depois de escolher a ROM)
Emulator::Emulator(): cpu(&memory){
    rendererInitialized = false;
}

bool Emulator::loadROM(const std::string& path){
    // Carrega ROM no barramento. Se falhar, não dá pra rodar.
    if (!memory.loadROM(path)) {
        return false;
    }

    // Reset da CPU: no 6502/6507 isso carrega o vetor de reset e inicia o boot.
    cpu.reset();

    // Inicializa o detector de frame.
    lastScanline = memory.tia.getScanline();
    return true;
}

void Emulator::step(){
    // Executa 1 instrução e avança o "mundo" pelo número real de ciclos.
    // Atari 2600 depende de sincronização por ciclo (o jogo desenha no timing).
    const uint64_t cyclesBefore = cpu.cycles;
    cpu.cpuClock();
    const uint64_t cyclesAfter = cpu.cycles;

    uint32_t cpuCyclesThisInstruction = 1;
    if (cyclesAfter > cyclesBefore) {
        cpuCyclesThisInstruction = static_cast<uint32_t>(cyclesAfter - cyclesBefore);
    }

    // Memory::step(cpuCycles) já faz TIA = 3 clocks por ciclo de CPU.
    memory.step(cpuCyclesThisInstruction);
}

bool Emulator::endOfFrame(){
    // - scanline vai de 0..261
    // - quando ela volta para 0 após estar em 261, tem um novo frame.
    int scan = memory.tia.getScanline();
    bool frame = (scan == 0 && lastScanline == 261);
    lastScanline = scan;
    return frame;
}

// Loop principal de emulação
void Emulator::run(){
    if (!rendererInitialized) {
        // Resolução nativa do frame:
        // - 160 pixels horizontais visíveis
        // - 262 scanlines (NTSC frame completo)
        // Aumentando a escala em 3x em X e Y para ficar maior na tela.
        if (!renderer.init(160, 262, 3, 3)) {
            std::cerr << "Falha ao inicializar renderer\n";
            return;
        }
        rendererInitialized = true;
    }

    // Configurações de debug via variáveis de ambiente.
    const char* venv = std::getenv("VERBOSE");
    cpu.verbose = (venv && venv[0] != '0');

    // TIA debug logs
    const char* tenv = std::getenv("TIA_DEBUG");
    memory.tia.setDebug(tenv && tenv[0] != '0');

    constexpr auto targetFrameTime = std::chrono::microseconds(16667); // ~60Hz

    while (true) {
        const auto frameStart = std::chrono::steady_clock::now();

        // Processa eventos da janela (fechar, ESC, etc).
        renderer.poll();

        // 1) Lê estado do teclado via SDL
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool fire = false;
        bool gameSelect = false;
        bool gameReset = false;
        bool left2 = false;
        bool right2 = false;
        bool up2 = false;
        bool down2 = false;
        bool fire2 = false;

        SDL_PumpEvents();

        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        // Player 0
        left  = keys[SDL_SCANCODE_LEFT]  != 0;
        right = keys[SDL_SCANCODE_RIGHT] != 0;
        up    = keys[SDL_SCANCODE_UP]    != 0;
        down  = keys[SDL_SCANCODE_DOWN]  != 0;
        fire  = keys[SDL_SCANCODE_SPACE] != 0;
        gameSelect = keys[SDL_SCANCODE_Z] != 0;
        gameReset  = keys[SDL_SCANCODE_X] != 0;
        // Player 1
        left2 = keys[SDL_SCANCODE_A] != 0;
        right2 = keys[SDL_SCANCODE_D] != 0;
        up2 = keys[SDL_SCANCODE_W] != 0;
        down2 = keys[SDL_SCANCODE_S] != 0;
        fire2 = keys[SDL_SCANCODE_LCTRL] != 0;

        // Joystick (SWCHA) (active low)
        // P0: bit7=Right, bit6=Left, bit5=Down, bit4=Up
        // P1: bit3=Right, bit2=Left, bit1=Down, bit0=Up
        uint8_t swcha = 0xFF;
        if (right) swcha &= static_cast<uint8_t>(~0x80);
        if (left)  swcha &= static_cast<uint8_t>(~0x40);
        if (down)  swcha &= static_cast<uint8_t>(~0x20);
        if (up)    swcha &= static_cast<uint8_t>(~0x10);
        if (right2) swcha &= static_cast<uint8_t>(~0x08);
        if (left2)  swcha &= static_cast<uint8_t>(~0x04);
        if (down2)  swcha &= static_cast<uint8_t>(~0x02);
        if (up2)    swcha &= static_cast<uint8_t>(~0x01);
        memory.riot.setSWCHA(swcha);

        // 4) Teclado -> Console switches (SWCHB) (active low)
        // SWCHB bit0 = RESET, bit1 = SELECT
        uint8_t swchb = 0xFF;
        if (gameReset)  swchb &= static_cast<uint8_t>(~0x01);
        if (gameSelect) swchb &= static_cast<uint8_t>(~0x02);
        memory.riot.setSWCHB(swchb);

        // 5) Teclado -> TIA inputs (triggers)
        memory.tia.setTrigger0Pressed(fire);
        memory.tia.setTrigger1Pressed(fire2);

        // Emula CPU+TIA até completar 1 frame inteiro.
        // Isso deixa o emulador bem mais rápido e reduz overhead de input/poll.
        while (!endOfFrame()) {
            step();
            if (renderer.shouldClose()) {
                break;
            }
        }

        renderer.present(memory.tia);

        // Throttle para ~60Hz
        const auto frameEnd = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
        if (elapsed < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - elapsed);
        }

        // Se a janela fechou, encerramos.
        if (renderer.shouldClose()) {
            break;
        }

        if (cpu.verbose) {
            cpu.dumpState();
        }
    }
}

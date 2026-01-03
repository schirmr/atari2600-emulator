#include "emulator.hpp"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <SDL2/SDL.h>
#endif

// Construtor:
// - Conecta a CPU no barramento (Memory)
// - Inicializa o renderer
Emulator::Emulator(): cpu(&memory){
    // Inicializamos o renderer aqui para o main ficar simples.
    //
    // Para testes de fidelidade do TIA, usarei a resolução nativa do frame:
    // - 160 pixels horizontais visíveis
    // - 262 scanlines (NTSC frame completo)
    // Escala 3x (pixel-perfect). A resolução emulada continua 160x262.
    if (!renderer.init(160, 262, 3, 3)) {
        std::cerr << "Falha ao inicializar renderer\n";
    }
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
    // Heurística simples:
    // - scanline vai de 0..261
    // - quando ela volta para 0 após estar em 261, consideramos um novo frame.
    int scan = memory.tia.getScanline();
    bool frame = (scan == 0 && lastScanline == 261);
    lastScanline = scan;
    return frame;
}

// Loop principal de emulação
void Emulator::run(){
    // Debug da CPU via variável de ambiente:
    // - VERBOSE=1 imprime dumpState a cada passo.
    const char* venv = std::getenv("VERBOSE");
    cpu.verbose = (venv && venv[0] != '0');

    // Debug do TIA via variável de ambiente:
    // - TIA_DEBUG=1 imprime logs quando COLUBK muda.
    const char* tenv = std::getenv("TIA_DEBUG");
    memory.tia.setDebug(tenv && tenv[0] != '0');

    // Limita a velocidade a ~60 FPS (NTSC).
    // Sem isso, o emulador roda no mximo do PC e o jogo fica acelerado.
    constexpr auto targetFrameTime = std::chrono::microseconds(16667); // ~60Hz

    while (true) {
        const auto frameStart = std::chrono::steady_clock::now();

        // Processa eventos da janela (fechar, ESC, etc).
        renderer.poll();

        // Le teclado (Win32 ou SDL) uma vez por frame.
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool fire = false;
        bool gameSelect = false;
        bool gameReset = false;

#ifdef _WIN32
        left  = (GetAsyncKeyState(VK_LEFT)  & 0x8000) != 0;
        right = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
        up    = (GetAsyncKeyState(VK_UP)    & 0x8000) != 0;
        down  = (GetAsyncKeyState(VK_DOWN)  & 0x8000) != 0;
        fire  = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        gameSelect = (GetAsyncKeyState('Z') & 0x8000) != 0;
        gameReset  = (GetAsyncKeyState('X') & 0x8000) != 0;
#else
        // SDL2: garante que o estado do teclado esteja atualizado
        SDL_PumpEvents();

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        left  = keys[SDL_SCANCODE_LEFT]  != 0;
        right = keys[SDL_SCANCODE_RIGHT] != 0;
        up    = keys[SDL_SCANCODE_UP]    != 0;
        down  = keys[SDL_SCANCODE_DOWN]  != 0;
        fire  = keys[SDL_SCANCODE_SPACE] != 0;
        gameSelect = keys[SDL_SCANCODE_Z] != 0;
        gameReset  = keys[SDL_SCANCODE_X] != 0;
#endif
        // 3) Teclado -> Joystick (SWCHA) (active low)
        // P0: bit7=Right, bit6=Left, bit5=Down, bit4=Up
        uint8_t swcha = 0xFF;
        if (right) swcha &= static_cast<uint8_t>(~0x80);
        if (left)  swcha &= static_cast<uint8_t>(~0x40);
        if (down)  swcha &= static_cast<uint8_t>(~0x20);
        if (up)    swcha &= static_cast<uint8_t>(~0x10);
        memory.riot.setSWCHA(swcha);

        // 4) Teclado -> Console switches (SWCHB) (active low)
        // SWCHB bit0 = RESET, bit1 = SELECT
        uint8_t swchb = 0xFF;
        if (gameReset)  swchb &= static_cast<uint8_t>(~0x01);
        if (gameSelect) swchb &= static_cast<uint8_t>(~0x02);
        memory.riot.setSWCHB(swchb);

        // 5) Teclado -> Trigger (botão de disparo) (INPT4/INPT5)
        // Space Invaders usa o controle 0 (porta esquerda) como jogador 1.
        // Mantemos o controle 1 solto para não confundir a lógica de 2 jogadores.
        memory.tia.setTrigger0Pressed(fire);
        memory.tia.setTrigger1Pressed(false);

        // Emula CPU+TIA at completar 1 frame inteiro.
        // Isso deixa o emulador bem mais rpido e reduz overhead de input/poll.
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

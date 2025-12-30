#include "emulator.hpp"
#include <iostream>
#include <cstdlib>

// ------------------------------
// Emulator (implementação)
// ------------------------------
//
// Aqui fica o "loop principal" do emulador.
// A ideia é manter o main.cpp minimalista e concentrar a complexidade aqui.

// Construtor:
// - Conecta a CPU no barramento (Memory)
// - Inicializa o renderer
Emulator::Emulator(): cpu(&memory){
    // Inicializamos o renderer aqui para o main ficar simples.
    //
    // Por que 160x192?
    // - 160 é uma largura comum para a área visível "lógica" (simplificação).
    // - 192 scanlines é um valor típico de área visível NTSC em muitos jogos.
    //
    // Por que scaleX=5 e scaleY=3?
    // - O Atari 2600 na TV fica em 4:3.
    // - Os pixels não são quadrados.
    // - Então fazemos uma correção de aspecto simples (escala X diferente de Y).
    if (!renderer.init(160, 192, 5, 3)) {
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
    // Roda a CPU (núcleo atual executa um "clock" / instrução por chamada).
    cpu.cpuClock();

    // Sincronização CPU <-> TIA:
    // No Atari 2600, o TIA roda 3x mais rápido que a CPU.
    // Nosso Memory::step(cycles) executa 3 clocks do TIA a cada 1 ciclo de CPU.
    memory.step(3);
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

    while (true) {
        // 1) Emula CPU+TIA por um passo.
        step();

        // 2) Processa eventos da janela (fechar, ESC, etc).
        renderer.poll();

        // 3) Só desenhamos quando um frame terminou.
        // Isso reduz custo e deixa a saída mais estável.
        if (endOfFrame()) {
            renderer.present(memory.tia);
        }

        // 4) Se a janela fechou, encerramos.
        if (renderer.shouldClose()) {
            break;
        }

        if (cpu.verbose) {
            cpu.dumpState();
        }
    }
}

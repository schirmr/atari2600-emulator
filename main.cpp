#include <iostream>
#include "./emulator/emulator.hpp"

int main() {
    Emulator emulator;
    if (!emulator.loadROM("./tests/pac_man.a26")) { // add o caminho para o jogo que quer rodar
        std::cerr << "Falha ao carregar ROM\n";
        return 1;
    }
    emulator.run();
    return 0;
}

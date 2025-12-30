#include <iostream>
#include "./emulator/emulator.hpp"

int main() {
    Emulator emulator;
    if (!emulator.loadROM("./tests/space_invaders.a26")) {
        std::cerr << "Falha ao carregar ROM\n";
        return 1;
    }
    emulator.run();
    return 0;
}

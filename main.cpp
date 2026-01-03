#include <iostream>
#include "./emulator/emulator.hpp"

#include "./ui/rom_picker.hpp"

int main() {
    RomPicker picker;
    const auto romPath = picker.pickRomFromTestsDir("./tests");
    if (!romPath) {
        return 0;
    }

    Emulator emulator;
    if (!emulator.loadROM(*romPath)) {
        std::cerr << "Falha ao carregar ROM\n";
        return 1;
    }

    emulator.run();
    return 0;
}

#pragma once

#include <cstdint>

// TIA Palette
//
// No Atari 2600, o TIA usa um código de cor de 8 bits. Em forma simplificada:
// - nibble alto (bits 7..4): "matiz" (hue) 0..15
// - nibble baixo (bits 3..0): "luminância" (luma) 0..15
//
// O hardware real + TV (NTSC/PAL) tem uma conversao analogica dificil.
// Neste projeto, eu uso uma aproximacao digital simples.

namespace tia_palette {

enum class Mode {
    NTSC,
    PAL,
};

struct Rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Converte um color code do TIA (0..255) para um RGB 0..255.
Rgb tiaColorToRgb(uint8_t code, Mode mode);

}

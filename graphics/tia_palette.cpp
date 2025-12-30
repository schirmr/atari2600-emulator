#include "tia_palette.hpp"

#include <cmath>

// Implementação da paleta do TIA.
// Importante: isso é aproximado (HSV). O TIA real gera um sinal analógico,
// e a cor final depende de TV, fase, filtros, etc.

namespace tia_palette {

// Converte um valor no intervalo [0.0, 1.0] para um byte [0, 255].
static uint8_t clamp01ToByte(double v01) {
    if (v01 < 0.0) {
        v01 = 0.0;
    }
    if (v01 > 1.0) {
        v01 = 1.0;
    }

    double scaled = v01 * 255.0;
    return static_cast<uint8_t>(std::lround(scaled));
}

// Aplicação de gamma simples.
static double applyGamma(double v, double gamma) {
    if (v <= 0.0) {
        return 0.0;
    }
    return std::pow(v, gamma);
}

// Conversão HSV -> RGB.
//
// HSV:
// - H: hue (matiz) em graus [0..360)
// - S: saturação [0..1]
// - V: valor/brilho [0..1]
static Rgb hsvToRgb(double hDeg, double s, double v) {
    // 1) Normaliza hue para [0, 360)
    while (hDeg < 0.0) {
        hDeg += 360.0;
    }
    while (hDeg >= 360.0) {
        hDeg -= 360.0;
    }

    // 2) Computa componentes intermediárias
    double c = v * s; // chroma
    double h = hDeg / 60.0;
    double x = c * (1.0 - std::fabs(std::fmod(h, 2.0) - 1.0));
    double m = v - c;

    // 3) Escolhe setor do círculo de cores
    double r1 = 0.0;
    double g1 = 0.0;
    double b1 = 0.0;

    if (h < 1.0) {
        r1 = c;
        g1 = x;
        b1 = 0.0;
    } else if (h < 2.0) {
        r1 = x;
        g1 = c;
        b1 = 0.0;
    } else if (h < 3.0) {
        r1 = 0.0;
        g1 = c;
        b1 = x;
    } else if (h < 4.0) {
        r1 = 0.0;
        g1 = x;
        b1 = c;
    } else if (h < 5.0) {
        r1 = x;
        g1 = 0.0;
        b1 = c;
    } else {
        r1 = c;
        g1 = 0.0;
        b1 = x;
    }

    // 4) Soma o offset m e converte para byte
    Rgb out;
    out.r = clamp01ToByte(r1 + m);
    out.g = clamp01ToByte(g1 + m);
    out.b = clamp01ToByte(b1 + m);
    return out;
}

Rgb tiaColorToRgb(uint8_t code, Mode mode) {
    // Separa nibbles
    // Exemplo: code = 0xC5 -> hueIdx = 0xC (12), lumIdx = 0x5 (5)
    int hueIdx = (code >> 4) & 0x0F;
    int lumIdx = code & 0x0F;

    // Mapeia hueIdx (0..15) para graus (0..360)
    // Cada passo = 360/16 = 22.5 graus.
    double hue = static_cast<double>(hueIdx) * (360.0 / 16.0);

    // Pequena diferença entre NTSC/PAL (aproximado).
    if (mode == Mode::PAL) {
        hue += 10.0;
    }

    // Saturação relativamente alta pro atari.
    double sat = 0.85;
    if (mode == Mode::PAL) {
        sat = 0.80;
    }

    // Luminância: lumIdx é 0..15.
    // evita o preto total (0) pra não desaparecer tudo quando lum=0.
    double v = static_cast<double>(lumIdx + 1) / 16.0; // 1/16..1
    v = 0.08 + v * 0.92;                               // sobe o piso
    v = applyGamma(v, 1.35);                           // ajusta médios

    return hsvToRgb(hue, sat, v);
}

}

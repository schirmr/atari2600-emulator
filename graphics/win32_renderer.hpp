#pragma once
#ifdef _WIN32 // Compila apenas se for windows, pois é um renderer Win32
#include <windows.h>
#include <cstdint>
#include <cstdlib>
#include "../tia/tia.hpp"
#include "tia_palette.hpp"

// obs: este renderer assume tamanho fixo na tela (sem resize/maximize/fullscreen)
// p/ manter o codigo simples nesta fase do projeto.
class Win32Renderer {
public:
    Win32Renderer() = default;
    ~Win32Renderer();

    // width/height: tamanho lógico do framebuffer
    // sx/sy: escala em X e Y (para correção de aspect ratio)
    bool init(int width, int height, int sx, int sy);
    void poll();
    bool shouldClose() const;
    void present(const Tia& tia);

private:
    HWND hwnd = nullptr;
    HDC hdc = nullptr;
    BITMAPINFO bmi{};
    uint8_t* dibPixels = nullptr;
    int fbW = 0, fbH = 0;
    int scaleX = 1;
    int scaleY = 1;

    int yOffset = 40;

    tia_palette::Mode paletteMode = tia_palette::Mode::NTSC;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static tia_palette::Mode readPaletteModeFromEnv();
    tia_palette::Rgb tiaColorToRGB(uint8_t code) const;
};
#endif

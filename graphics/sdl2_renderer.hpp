#pragma once

// ------------------------------
// SDL2 Renderer (Windows/Linux)
// ------------------------------

#include <cstdint>
#include <vector>

#include "../tia/tia.hpp"
#include "tia_palette.hpp"

// Forward declarations (evita puxar SDL.h no header)
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class Sdl2Renderer {
public:
    // Cria janela + renderer + texture.
    // width/height: tamanho lógico do framebuffer.
    // scaleX/scaleY: escala em X e Y (correção de aspect ratio).
    bool init(int width, int height, int scaleX, int scaleY);

    // Processa eventos (fechar janela, ESC).
    void poll();

    // Se true, o loop principal deve encerrar.
    bool shouldClose() const;

    // Copia o framebuffer do TIA para a textura e apresenta na janela.
    void present(const Tia& tia);

    // Libera recursos SDL.
    ~Sdl2Renderer();

private:
    // Ponteiros SDL. A criação/destruição acontece em init() / ~Sdl2Renderer().
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    int fbW = 0;
    int fbH = 0;
    int scaleX = 1;
    int scaleY = 1;

    int yOffset = 0;

    bool quit = false;
    bool sdlInitialized = false;

    tia_palette::Mode paletteMode = tia_palette::Mode::NTSC;
    // Buffer de pixels em CPU (ARGB8888). Depois copiamos para a SDL_Texture.
    std::vector<uint32_t> pixels;
};

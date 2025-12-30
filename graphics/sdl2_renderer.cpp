#ifndef _WIN32

#include "sdl2_renderer.hpp"

#include <SDL.h>
#include <cstdlib>

// ------------------------------
// SDL2 Renderer (implementação)
// ------------------------------

static tia_palette::Mode readPaletteModeFromEnv() {
    const char* env = std::getenv("TIA_PALETTE");
    if (!env) return tia_palette::Mode::NTSC;
    if ((env[0] == 'P' || env[0] == 'p') && (env[1] == 'A' || env[1] == 'a')) {
        return tia_palette::Mode::PAL;
    }
    return tia_palette::Mode::NTSC;
}

bool Sdl2Renderer::init(int width, int height, int scaleX, int scaleY) {
    // Evita reinicializar
    if (window != nullptr) {
        return true;
    }

    fbW = width;
    fbH = height;

    if (scaleX <= 0) {
        this->scaleX = 1;
    } else {
        this->scaleX = scaleX;
    }

    if (scaleY <= 0) {
        this->scaleY = 1;
    } else {
        this->scaleY = scaleY;
    }

    paletteMode = readPaletteModeFromEnv();

    if (!sdlInitialized) {
        // Inicializa o subsistema de vídeo do SDL.
        // SDL_Init retorna 0 em sucesso.
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            return false;
        }
        sdlInitialized = true;
    }

    window = SDL_CreateWindow(
        "Atari 2600",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        fbW * this->scaleX,
        fbH * this->scaleY,
        SDL_WINDOW_SHOWN
    );
    if (!window) return false;

    // Bloqueia resize/maximize/fullscreen via WM: nosso pipeline assume tamanho fixo.
    SDL_SetWindowResizable(window, SDL_FALSE);
#if SDL_VERSION_ATLEAST(2, 0, 5)
    SDL_SetWindowMinimumSize(window, fbW * this->scaleX, fbH * this->scaleY);
    SDL_SetWindowMaximumSize(window, fbW * this->scaleX, fbH * this->scaleY);
#endif

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        fbW,
        fbH
    );
    if (!texture) return false;

    pixels.clear();
    pixels.resize(static_cast<size_t>(fbW) * static_cast<size_t>(fbH));
    return true;
}

void Sdl2Renderer::poll() {
    if (!sdlInitialized) return;
    if (!window) return;

    // SDL_PollEvent tira eventos da fila.
    // Aqui trata só:
    // - fechar janela
    // - ESC para sair
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            quit = true;
        }
    }
}

bool Sdl2Renderer::shouldClose() const {
    if (!window) return true;
    return quit;
}

void Sdl2Renderer::present(const Tia& tia) {
    if (!window) return;
    if (!renderer) return;
    if (!texture) return;

    // 1) Preenche o buffer ARGB8888 (CPU) a partir do buffer do TIA.
    for (int y = 0; y < fbH; ++y) {
        const uint8_t* row = tia.getScanlineBuffer(y + yOffset);
        for (int x = 0; x < fbW; ++x) {
            const uint8_t code = row ? row[x] : 0;
            tia_palette::Rgb rgb = tia_palette::tiaColorToRgb(code, paletteMode);

            uint32_t argb = 0;
            argb |= (0xFFu << 24);
            argb |= (static_cast<uint32_t>(rgb.r) << 16);
            argb |= (static_cast<uint32_t>(rgb.g) << 8);
            argb |= (static_cast<uint32_t>(rgb.b) << 0);

            pixels[static_cast<size_t>(y) * static_cast<size_t>(fbW) + static_cast<size_t>(x)] = argb;
        }
    }

    // 2) Copia pixels -> textura da GPU.
    // pitch = bytes por linha = fbW * 4 (ARGB8888).
    SDL_UpdateTexture(texture, nullptr, pixels.data(), fbW * 4);

    // 3) Limpa e desenha a textura na janela.
    SDL_RenderClear(renderer);

    SDL_Rect dst;
    dst.x = 0;
    dst.y = 0;
    dst.w = fbW * this->scaleX;
    dst.h = fbH * this->scaleY;

    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_RenderPresent(renderer);
}

Sdl2Renderer::~Sdl2Renderer() {
    // Ordem de destruição típica:
    // texture -> renderer -> window
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    if (sdlInitialized) {
        // SDL_Quit encerra todos subsistemas do SDL.
        SDL_Quit();
        sdlInitialized = false;
    }
}

#endif

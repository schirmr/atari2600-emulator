#include "rom_picker.hpp"

#include "mini_font.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

namespace {

std::string toUpperAscii(std::string s) {
    for (char& ch : s) {
        unsigned char u = static_cast<unsigned char>(ch);
        ch = static_cast<char>(std::toupper(u));
    }
    return s;
}

bool hasA26Extension(const std::filesystem::path& p) {
    if (!p.has_extension()) return false;
    std::string ext = p.extension().string();
    for (char& c : ext) {
        unsigned char u = static_cast<unsigned char>(c);
        c = static_cast<char>(std::tolower(u));
    }
    return ext == ".a26";
}

std::vector<std::filesystem::path> listRoms(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> out;
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) return out;

    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;
        const auto& p = entry.path();
        if (hasA26Extension(p)) {
            out.push_back(p);
        }
    }

    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return a.filename().string() < b.filename().string();
    });

    return out;
}

void drawChar(SDL_Renderer* r, char c, int x, int y, int scale) {
    const mini_font::Glyph g = mini_font::glyphFor(c);
    for (int row = 0; row < 7; ++row) {
        const uint8_t bits = g.rows[row];
        for (int col = 0; col < 5; ++col) {
            const bool on = ((bits >> (4 - col)) & 0x1) != 0;
            if (!on) continue;

            SDL_Rect px;
            px.x = x + col * scale;
            px.y = y + row * scale;
            px.w = scale;
            px.h = scale;
            SDL_RenderFillRect(r, &px);
        }
    }
}

void drawText(SDL_Renderer* r, const std::string& text, int x, int y, int scale) {
    int cx = x;
    for (char c : text) {
        drawChar(r, c, cx, y, scale);
        cx += (5 + 1) * scale;
    }
}

} // namespace

std::optional<std::string> RomPicker::pickRomFromTestsDir(const std::string& testsDir) {
    const std::filesystem::path dir = std::filesystem::path(testsDir);
    std::vector<std::filesystem::path> roms = listRoms(dir);

    if (roms.empty()) {
        return std::nullopt;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return std::nullopt;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Select ROM",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Quit();
        return std::nullopt;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return std::nullopt;
    }

    int selected = 0;
    int scroll = 0;

    const int scale = 2;
    const int lineH = 9 * scale; 
    const int leftPad = 12;
    const int topPad = 12;

    bool quit = false;
    bool choose = false;

    while (!quit && !choose) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    choose = true;
                } else if (e.key.keysym.sym == SDLK_UP) {
                    if (selected > 0) selected--;
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (selected + 1 < static_cast<int>(roms.size())) selected++;
                } else if (e.key.keysym.sym == SDLK_PAGEUP) {
                    selected -= 10;
                    if (selected < 0) selected = 0;
                } else if (e.key.keysym.sym == SDLK_PAGEDOWN) {
                    selected += 10;
                    if (selected >= static_cast<int>(roms.size())) selected = static_cast<int>(roms.size()) - 1;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0 && selected > 0) selected--;
                if (e.wheel.y < 0 && selected + 1 < static_cast<int>(roms.size())) selected++;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                const int mx = e.button.x;
                const int my = e.button.y;
                (void)mx;

                const int idx = (my - topPad) / lineH + scroll;
                if (idx >= 0 && idx < static_cast<int>(roms.size())) {
                    selected = idx;
                    choose = true;
                }
            }
        }

        int w = 0, h = 0;
        SDL_GetWindowSize(window, &w, &h);
        const int visibleLines = std::max(1, (h - topPad * 2) / lineH);

        if (selected < scroll) scroll = selected;
        if (selected >= scroll + visibleLines) scroll = selected - visibleLines + 1;
        if (scroll < 0) scroll = 0;

        const int maxScroll = std::max(0, static_cast<int>(roms.size()) - visibleLines);
        if (scroll > maxScroll) scroll = maxScroll;

        // Background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // List
        for (int i = 0; i < visibleLines; ++i) {
            const int idx = scroll + i;
            if (idx >= static_cast<int>(roms.size())) break;

            const int y = topPad + i * lineH;
            const bool isSel = (idx == selected);

            if (isSel) {
                SDL_Rect bg{leftPad - 6, y - 2, w - (leftPad - 6) - 12, lineH};
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                SDL_RenderFillRect(renderer, &bg);
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            std::string name = roms[idx].filename().string();
            name = toUpperAscii(name);
            drawText(renderer, name, leftPad, y, scale);
        }

        SDL_RenderPresent(renderer);
    }

    std::optional<std::string> result;
    if (choose && !quit && selected >= 0 && selected < static_cast<int>(roms.size())) {
        result = roms[selected].string();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return result;
}

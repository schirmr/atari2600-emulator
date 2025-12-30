#ifdef _WIN32
#include "win32_renderer.hpp"

// WndProc (Windows Procedure): trata mensagens da janela (fechar, redimensionar, etc)
LRESULT CALLBACK Win32Renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    (void)lParam;

    if (msg == WM_DESTROY) { // Quando a janela for fechada
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Paleta via variável de ambiente

tia_palette::Mode Win32Renderer::readPaletteModeFromEnv() {
    const char* env = std::getenv("TIA_PALETTE");
    if (env == nullptr) {
        return tia_palette::Mode::NTSC;
    }

    // aceita: NTSC / PAL
    if ((env[0] == 'P' || env[0] == 'p') && (env[1] == 'A' || env[1] == 'a')) {
        return tia_palette::Mode::PAL;
    }

    return tia_palette::Mode::NTSC;
}

tia_palette::Rgb Win32Renderer::tiaColorToRGB(uint8_t code) const {
    return tia_palette::tiaColorToRgb(code, paletteMode);
}

bool Win32Renderer::init(int width, int height, int sx, int sy) {
    fbW = width;
    fbH = height;

    if (sx <= 0) scaleX = 1;
    else scaleX = sx;

    if (sy <= 0) scaleY = 1;
    else scaleY = sy;

    paletteMode = readPaletteModeFromEnv();

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.lpfnWndProc = Win32Renderer::WndProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "TIARendererClass";
    wc.hCursor = LoadCursorA(nullptr, (LPCSTR)IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        return false;
    }

    const DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    const DWORD exStyle = 0;

    // Tamanho do client area (área interna onde desenhamos).
    const int clientW = fbW * scaleX;
    const int clientH = fbH * scaleY;

    // Ajusta o tamanho total da janela (inclui bordas + barra de titulo)
    RECT r;
    r.left = 0;
    r.top = 0;
    r.right = clientW;
    r.bottom = clientH;
    AdjustWindowRectEx(&r, style, FALSE, exStyle);

    hwnd = CreateWindowExA(
        exStyle,
        wc.lpszClassName,
        "Atari 2600",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        r.right - r.left,
        r.bottom - r.top,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (hwnd == nullptr) {
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);

    hdc = GetDC(hwnd);
    if (hdc == nullptr) {
        return false;
    }

    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = clientW;
    bmi.bmiHeader.biHeight = -clientH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    // Aloca memória para o DIB (32 bits: BGRA)
    dibPixels = (uint8_t*)std::malloc(static_cast<size_t>(clientW) * static_cast<size_t>(clientH) * 4u);
    if (dibPixels == nullptr) {
        return false;
    }

    return true;
}

void Win32Renderer::poll() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Win32Renderer::shouldClose() const {
    return !IsWindow(hwnd); // retorna false se a janela foi fechada
}

// Converte o buffer do TIA para o nosso dibPixels (32-bit) e copia pra janela.
void Win32Renderer::present(const Tia& tia) {
    // Tamanho em pixels do client area.
    int winW = fbW * scaleX;
    int winH = fbH * scaleY;

    // Para cada pixel lógico (x,y) do framebuffer, pintamos um bloco scaleX x scaleY.
    for (int y = 0; y < fbH; ++y) {
        const uint8_t* row = tia.getScanlineBuffer(y + yOffset);

        for (int x = 0; x < fbW; ++x) {
            uint8_t tiaColor = 0;
            if (row != nullptr) {
                tiaColor = row[x];
            }

            tia_palette::Rgb rgb = tiaColorToRGB(tiaColor);

            // Desenha o pixel escalado
            for (int yy = 0; yy < scaleY; ++yy) {
                for (int xx = 0; xx < scaleX; ++xx) {
                    int dstX = x * scaleX + xx;
                    int dstY = y * scaleY + yy;

                    // dibPixels é BGRA (ordem de bytes)
                    int idx = (dstY * winW + dstX) * 4;
                    dibPixels[idx + 0] = rgb.b;
                    dibPixels[idx + 1] = rgb.g;
                    dibPixels[idx + 2] = rgb.r;
                    dibPixels[idx + 3] = 0xFF;
                }
            }
        }
    }

    // StretchDIBits: copia o buffer para o device context da janela.
    StretchDIBits(
        hdc,
        0,
        0,
        winW,
        winH,
        0,
        0,
        winW,
        winH,
        dibPixels,
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

// Destructor
// Libera recursos do Windows e memória.
Win32Renderer::~Win32Renderer() {
    if (hwnd != nullptr && hdc != nullptr) {
        ReleaseDC(hwnd, hdc);
        hdc = nullptr;
    }

    if (hwnd != nullptr) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }

    if (dibPixels != nullptr) {
        std::free(dibPixels);
        dibPixels = nullptr;
    }
}

#endif

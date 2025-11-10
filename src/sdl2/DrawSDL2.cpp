// ---------------------------------------------------------------------------
//  DrawSDL2 - SDL2-based drawing implementation for M88
// ---------------------------------------------------------------------------

#include "DrawSDL2.h"
#include <stdio.h>
#include <string.h>

DrawSDL2::DrawSDL2()
    : window(nullptr)
    , renderer(nullptr)
    , texture(nullptr)
    , framebuffer(nullptr)
    , width(0)
    , height(0)
    , bpp(0)
    , pitch(0)
    , status_flags(0)
    , flip_mode(false)
    , locked(false)
    , palette_changed(false)
{
    // デフォルトパレット（グレースケール）
    for (int i = 0; i < 256; i++) {
        palette[i].r = i;
        palette[i].g = i;
        palette[i].b = i;
        palette[i].a = 255;
    }
}

DrawSDL2::~DrawSDL2()
{
    Cleanup();
}

bool DrawSDL2::Init(uint w, uint h, uint bits)
{
    width = w;
    height = h;
    bpp = bits;

    printf("[DrawSDL2] Initializing: %dx%d, %dbpp\n", width, height, bpp);

    // SDL2 Videoは main() で既に初期化済み
    // SDL_InitSubSystem(SDL_INIT_VIDEO) は不要（冗長）

    // テスト用パレット設定（ROMなしでも見えるように）
    for (int i = 0; i < 8; i++) {
        palette[i].r = (i & 1) ? 255 : 0;  // Red
        palette[i].g = (i & 2) ? 255 : 0;  // Green
        palette[i].b = (i & 4) ? 255 : 0;  // Blue
        palette[i].a = 255;
    }
    // 白
    palette[15].r = palette[15].g = palette[15].b = palette[15].a = 255;

    // ウィンドウ作成
    window = SDL_CreateWindow(
        "M88 - PC-8801 Emulator (SDL2)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "[DrawSDL2] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // レンダラー作成
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        fprintf(stderr, "[DrawSDL2] SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        return false;
    }

    // テクスチャ作成（RGB888フォーマット）
    // PC-8801は8bppですが、後でパレット変換して24bitで描画します
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );

    if (!texture) {
        fprintf(stderr, "[DrawSDL2] SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        renderer = nullptr;
        window = nullptr;
        return false;
    }

    // フレームバッファ確保（8bpp用）
    pitch = width;  // 8bpp なので width と同じ
    framebuffer = new uint8[width * height];
    memset(framebuffer, 0, width * height);

    // テスト用: カラーバーを描画（ROMなしでも動作確認できるように）
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color_index = (x * 8) / width;  // 0-7の8色
            framebuffer[y * pitch + x] = color_index;
        }
    }

    status_flags = readytodraw | shouldrefresh;

    printf("[DrawSDL2] Initialization successful\n");
    printf("[DrawSDL2] TEST: Color bars drawn to framebuffer\n");
    return true;
}

bool DrawSDL2::Cleanup()
{
    if (framebuffer) {
        delete[] framebuffer;
        framebuffer = nullptr;
    }

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

    // SDL2 Videoの終了は main() で SDL_Quit() によって行われる
    // SDL_QuitSubSystem(SDL_INIT_VIDEO) は不要

    printf("[DrawSDL2] Cleanup complete\n");
    return true;
}

bool DrawSDL2::Lock(uint8** pimage, int* pbpl)
{
    if (!framebuffer) {
        fprintf(stderr, "[DrawSDL2] Lock failed: framebuffer is null\n");
        return false;
    }

    *pimage = framebuffer;
    *pbpl = pitch;
    locked = true;

    return true;
}

bool DrawSDL2::Unlock()
{
    locked = false;
    return true;
}

uint DrawSDL2::GetStatus()
{
    return status_flags;
}

void DrawSDL2::Resize(uint w, uint h)
{
    // TODO: リサイズ対応
    printf("[DrawSDL2] Resize requested: %dx%d (not implemented yet)\n", w, h);
}

void DrawSDL2::DrawScreen(const Region& region)
{
    if (!texture || !framebuffer) {
        fprintf(stderr, "[DrawSDL2] DrawScreen skipped: texture=%p framebuffer=%p\n", texture, framebuffer);
        return;
    }

    // SDL2テクスチャをロック
    void* pixels;
    int texture_pitch;

    if (SDL_LockTexture(texture, nullptr, &pixels, &texture_pitch) < 0) {
        fprintf(stderr, "[DrawSDL2] SDL_LockTexture failed: %s\n", SDL_GetError());
        return;
    }

    // フレームバッファ（8bpp）をRGB888に変換してテクスチャにコピー
    uint32_t* dest = (uint32_t*)pixels;
    int dest_pitch_pixels = texture_pitch / 4;

    // デバッグ: 最初の数ピクセルをチェック
    bool has_nonzero = false;
    for (int i = 0; i < 100 && i < width * height; i++) {
        if (framebuffer[i] != 0) {
            has_nonzero = true;
            break;
        }
    }
    if (!has_nonzero) {
        printf("[DrawSDL2] Warning: First 100 pixels are all zero (black)\n");
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8 index = framebuffer[y * pitch + x];
            SDL_Color& col = palette[index];

            uint32_t rgb = (col.r << 16) | (col.g << 8) | col.b;
            dest[y * dest_pitch_pixels + x] = rgb;
        }
    }

    SDL_UnlockTexture(texture);

    // レンダリング
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    status_flags &= ~shouldrefresh;
}

void DrawSDL2::SetPalette(uint index, uint nents, const Palette* pal)
{
    if (!pal) {
        return;
    }

    printf("[DrawSDL2] SetPalette: index=%u, nents=%u\n", index, nents);

    for (uint i = 0; i < nents; i++) {
        uint idx = index + i;
        if (idx < 256) {
            palette[idx].r = pal[i].red;
            palette[idx].g = pal[i].green;
            palette[idx].b = pal[i].blue;
            palette[idx].a = 255;

            // デバッグ: 最初の数色を表示
            if (i < 8) {
                printf("  Palette[%u] = RGB(%u, %u, %u)\n", idx, pal[i].red, pal[i].green, pal[i].blue);
            }
        }
    }

    palette_changed = true;
    status_flags |= shouldrefresh;
}

void DrawSDL2::Flip()
{
    // VSync対応済みなので、特に何もしない
}

bool DrawSDL2::SetFlipMode(bool flip)
{
    flip_mode = flip;
    return true;
}

bool DrawSDL2::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return false;
            }
        }
    }
    return true;
}

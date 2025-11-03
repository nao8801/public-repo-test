// SDL2 Graphics Test for M88 Prototype
// PC-8801mk2SR の画面解像度: 640x400

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 400;
const char* WINDOW_TITLE = "M88 SDL2 Graphics Test - PC-8801 Screen";

int main(int argc, char* argv[]) {
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    // ウィンドウ作成
    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "ウィンドウ作成失敗: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // レンダラー作成
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        fprintf(stderr, "レンダラー作成失敗: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // テクスチャ作成（フレームバッファとして使用）
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    if (!texture) {
        fprintf(stderr, "テクスチャ作成失敗: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("SDL2 グラフィックステスト起動\n");
    printf("画面サイズ: %dx%d (PC-8801mk2SR相当)\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    printf("ESCキーで終了\n\n");

    // フレームバッファ（ピクセルデータ）
    uint32_t* pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];

    bool running = true;
    SDL_Event event;
    int frame = 0;

    while (running) {
        // イベント処理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // フレームバッファを更新（テストパターン描画）
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                int index = y * SCREEN_WIDTH + x;

                // カラフルなテストパターン
                // PC-8801風の8色パレットっぽい雰囲気
                uint8_t r = ((x + frame) % 256);
                uint8_t g = ((y + frame / 2) % 256);
                uint8_t b = ((x + y + frame) % 256);

                // グラデーションパターン
                if (y < SCREEN_HEIGHT / 8) {
                    // 上部: 黒
                    r = g = b = 0;
                } else if (y < SCREEN_HEIGHT * 2 / 8) {
                    // 青
                    r = 0; g = 0; b = 255;
                } else if (y < SCREEN_HEIGHT * 3 / 8) {
                    // 赤
                    r = 255; g = 0; b = 0;
                } else if (y < SCREEN_HEIGHT * 4 / 8) {
                    // マゼンタ
                    r = 255; g = 0; b = 255;
                } else if (y < SCREEN_HEIGHT * 5 / 8) {
                    // 緑
                    r = 0; g = 255; b = 0;
                } else if (y < SCREEN_HEIGHT * 6 / 8) {
                    // シアン
                    r = 0; g = 255; b = 255;
                } else if (y < SCREEN_HEIGHT * 7 / 8) {
                    // 黄
                    r = 255; g = 255; b = 0;
                } else {
                    // 白
                    r = 255; g = 255; b = 255;
                }

                // チェッカーパターンを追加
                if (((x / 32) + (y / 32)) % 2 == 0) {
                    r = (r * 3) / 4;
                    g = (g * 3) / 4;
                    b = (b * 3) / 4;
                }

                pixels[index] = (r << 16) | (g << 8) | b;
            }
        }

        // テクスチャを更新
        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));

        // レンダリング
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame++;

        // フレームレート制御（約60fps）
        SDL_Delay(16);
    }

    // クリーンアップ
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("終了しました\n");
    return 0;
}

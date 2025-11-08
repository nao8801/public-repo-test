// Test program for DrawSDL2
// VRAMにテストパターンを書き込んで、DrawSDL2で表示する

#include "../../src/sdl2/DrawSDL2.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 400;
const int BPP = 8;  // 8bpp (indexed color)

// テストパターン描画
void DrawTestPattern(uint8_t* vram, int width, int height, int pitch, int frame)
{
    // カラーバーとチェッカーパターン
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t color = 0;

            if (y < height / 8) {
                // 黒
                color = 0;
            } else if (y < height * 2 / 8) {
                // 青
                color = 1;
            } else if (y < height * 3 / 8) {
                // 赤
                color = 2;
            } else if (y < height * 4 / 8) {
                // マゼンタ
                color = 3;
            } else if (y < height * 5 / 8) {
                // 緑
                color = 4;
            } else if (y < height * 6 / 8) {
                // シアン
                color = 5;
            } else if (y < height * 7 / 8) {
                // 黄
                color = 6;
            } else {
                // 白
                color = 7;
            }

            // チェッカーパターンを追加
            if (((x / 32) + (y / 32)) % 2 == 0) {
                color = (color + frame / 10) % 8;
            }

            vram[y * pitch + x] = color;
        }
    }

    // ボーダー描画（PC-8801風）
    for (int x = 0; x < width; x++) {
        vram[0 * pitch + x] = 7;  // 上
        vram[(height - 1) * pitch + x] = 7;  // 下
    }
    for (int y = 0; y < height; y++) {
        vram[y * pitch + 0] = 7;  // 左
        vram[y * pitch + (width - 1)] = 7;  // 右
    }
}

int main(int argc, char* argv[])
{
    printf("==============================================\n");
    printf("DrawSDL2 Test - PC-8801 Display Test\n");
    printf("==============================================\n\n");

    // DrawSDL2インスタンス作成
    DrawSDL2 draw;

    // 初期化
    if (!draw.Init(SCREEN_WIDTH, SCREEN_HEIGHT, BPP)) {
        fprintf(stderr, "DrawSDL2::Init failed\n");
        return 1;
    }

    // PC-8801の8色パレット設定
    Draw::Palette pc88_palette[8] = {
        {0x00, 0x00, 0x00, 0},  // 0: 黒
        {0x00, 0x00, 0xFF, 0},  // 1: 青
        {0xFF, 0x00, 0x00, 0},  // 2: 赤
        {0xFF, 0x00, 0xFF, 0},  // 3: マゼンタ
        {0x00, 0xFF, 0x00, 0},  // 4: 緑
        {0x00, 0xFF, 0xFF, 0},  // 5: シアン
        {0xFF, 0xFF, 0x00, 0},  // 6: 黄
        {0xFF, 0xFF, 0xFF, 0},  // 7: 白
    };

    draw.SetPalette(0, 8, pc88_palette);

    printf("DrawSDL2 initialized successfully\n");
    printf("Screen: %dx%d, %dbpp\n", SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
    printf("\n");
    printf("Controls:\n");
    printf("  ESC: Exit\n");
    printf("\n");
    printf("Displaying test pattern...\n");

    bool running = true;
    int frame = 0;

    while (running) {
        // イベント処理
        if (!draw.ProcessEvents()) {
            running = false;
            break;
        }

        // VRAMをロック
        uint8_t* vram;
        int pitch;

        if (!draw.Lock(&vram, &pitch)) {
            fprintf(stderr, "Lock failed\n");
            break;
        }

        // テストパターン描画
        DrawTestPattern(vram, SCREEN_WIDTH, SCREEN_HEIGHT, pitch, frame);

        // アンロック
        draw.Unlock();

        // 画面更新
        Draw::Region region;
        region.left = 0;
        region.top = 0;
        region.right = SCREEN_WIDTH;
        region.bottom = SCREEN_HEIGHT;

        draw.DrawScreen(region);

        frame++;

        // フレームレート表示（60フレームごと）
        if (frame % 60 == 0) {
            printf("Frame: %d\n", frame);
        }

        // 適度にウェイト（CPU負荷軽減）
        SDL_Delay(16);  // 約60fps
    }

    // クリーンアップ
    draw.Cleanup();

    printf("\nTest finished. Total frames: %d\n", frame);
    return 0;
}

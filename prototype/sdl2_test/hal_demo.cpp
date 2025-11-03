// HAL Integration Demo - HAL層を使った統合デモプログラム
//
// このデモでは、HAL抽象化レイヤーを使って、
// プラットフォーム非依存のエミュレータコードがどのように動作するかを示します。

#include "hal_prototype.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// プラットフォーム実装の作成関数（hal_sdl2_impl.cppで定義）
extern PlatformHAL* CreateSDL2Platform();

// ============================================================================
// Emulator Core - エミュレータコア（プラットフォーム非依存）
// ============================================================================

class EmulatorCore {
private:
    PlatformHAL* platform;
    bool running;
    int frameCount;

    // PC-8801 の仕様
    static const int SCREEN_WIDTH = 640;
    static const int SCREEN_HEIGHT = 400;
    static const int TARGET_FPS = 60;

public:
    EmulatorCore(PlatformHAL* plat) : platform(plat), running(false), frameCount(0) {}

    bool Initialize() {
        // グラフィックス初期化
        GraphicsHAL* gfx = platform->GetGraphics();
        if (!gfx->Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, "M88 HAL Demo - PC-8801 Emulator")) {
            fprintf(stderr, "Graphics initialization failed\n");
            return false;
        }

        // オーディオ初期化
        AudioHAL* audio = platform->GetAudio();
        if (!audio->Initialize(44100, 2, 4096)) {
            fprintf(stderr, "Audio initialization failed\n");
            return false;
        }
        audio->Play();

        // PC-8801風のパレット設定
        gfx->SetPalette(0, 0x00, 0x00, 0x00);  // 黒
        gfx->SetPalette(1, 0x00, 0x00, 0xFF);  // 青
        gfx->SetPalette(2, 0xFF, 0x00, 0x00);  // 赤
        gfx->SetPalette(3, 0xFF, 0x00, 0xFF);  // マゼンタ
        gfx->SetPalette(4, 0x00, 0xFF, 0x00);  // 緑
        gfx->SetPalette(5, 0x00, 0xFF, 0xFF);  // シアン
        gfx->SetPalette(6, 0xFF, 0xFF, 0x00);  // 黄
        gfx->SetPalette(7, 0xFF, 0xFF, 0xFF);  // 白

        printf("\n");
        printf("============================================\n");
        printf("  M88 HAL Integration Demo\n");
        printf("  PC-8801mk2SR Emulator Prototype\n");
        printf("============================================\n");
        printf("\n");
        printf("画面サイズ: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
        printf("フレームレート: %d FPS\n", TARGET_FPS);
        printf("\n");
        printf("ESCキーで終了\n");
        printf("\n");

        running = true;
        return true;
    }

    void RunFrame() {
        GraphicsHAL* gfx = platform->GetGraphics();
        TimerHAL* timer = platform->GetTimer();

        // フレームバッファロック
        uint32_t* pixels;
        int pitch;

        if (!gfx->LockFramebuffer(&pixels, &pitch)) {
            fprintf(stderr, "Failed to lock framebuffer\n");
            running = false;
            return;
        }

        // フレームバッファを描画
        // これは実際のPC-8801のVRAMをエミュレートする部分に相当
        DrawTestPattern(pixels, pitch);

        // フレームバッファアンロック
        gfx->UnlockFramebuffer();

        // 画面更新
        gfx->Present();

        frameCount++;
    }

    void DrawTestPattern(uint32_t* pixels, int pitch) {
        // PC-8801風のテストパターンを描画
        int pixelsPerRow = pitch / sizeof(uint32_t);

        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t r, g, b;

                // レトロなテストパターン
                if (y < 50) {
                    // タイトルバー風（シアン）
                    r = 0x00; g = 0xFF; b = 0xFF;
                } else if (y < 100) {
                    // グラデーション
                    r = (x * 255) / SCREEN_WIDTH;
                    g = 0;
                    b = ((SCREEN_WIDTH - x) * 255) / SCREEN_WIDTH;
                } else if (y < 200) {
                    // 8色パレット表示
                    int colorIndex = (x * 8) / SCREEN_WIDTH;
                    uint32_t colors[] = {
                        0x000000, 0x0000FF, 0xFF0000, 0xFF00FF,
                        0x00FF00, 0x00FFFF, 0xFFFF00, 0xFFFFFF
                    };
                    uint32_t color = colors[colorIndex];
                    r = (color >> 16) & 0xFF;
                    g = (color >> 8) & 0xFF;
                    b = color & 0xFF;
                } else if (y < 300) {
                    // チェッカーパターン
                    if (((x / 32) + (y / 32)) % 2 == 0) {
                        r = 0xFF; g = 0xFF; b = 0xFF;
                    } else {
                        r = 0x00; g = 0x00; b = 0x00;
                    }
                } else {
                    // アニメーションパターン
                    int dx = x - SCREEN_WIDTH / 2;
                    int dy = y - SCREEN_HEIGHT / 2;
                    double dist = sqrt(dx * dx + dy * dy);
                    double angle = atan2(dy, dx);

                    int wave = (int)(128 + 127 * sin(dist / 20.0 - frameCount / 10.0));
                    int rotate = (int)(128 + 127 * sin(angle * 5 + frameCount / 30.0));

                    r = wave;
                    g = rotate;
                    b = (wave + rotate) / 2;
                }

                pixels[y * pixelsPerRow + x] = (r << 16) | (g << 8) | b;
            }
        }

        // FPS表示エリア（左上）
        DrawText(pixels, pixelsPerRow, 10, 10, "M88 HAL DEMO");
        DrawText(pixels, pixelsPerRow, 10, 30, "PC-8801 PROTOTYPE");

        char fps_text[64];
        snprintf(fps_text, sizeof(fps_text), "FRAME: %d", frameCount);
        DrawText(pixels, pixelsPerRow, 10, SCREEN_HEIGHT - 30, fps_text);
    }

    // 簡易的な文字描画（8x8ドットフォント風）
    void DrawText(uint32_t* pixels, int pitch, int x, int y, const char* text) {
        // 超シンプルな実装：白い点で文字っぽいものを描く
        for (int i = 0; text[i] != '\0'; i++) {
            for (int dy = 0; dy < 8; dy++) {
                for (int dx = 0; dx < 6; dx++) {
                    int px = x + i * 8 + dx;
                    int py = y + dy;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                        // 簡易的なパターン（実際はフォントデータが必要）
                        if ((dx == 0 || dx == 5 || dy == 0 || dy == 7)) {
                            pixels[py * pitch + px] = 0xFFFFFF;
                        }
                    }
                }
            }
        }
    }

    void Run() {
        const int FRAME_TIME_MS = 1000 / TARGET_FPS;
        TimerHAL* timer = platform->GetTimer();

        while (running) {
            uint32_t frameStart = timer->GetMilliseconds();

            // イベント処理
            if (!platform->ProcessEvents()) {
                running = false;
                break;
            }

            // 1フレーム実行
            RunFrame();

            // フレームレート制御
            uint32_t frameEnd = timer->GetMilliseconds();
            uint32_t elapsed = frameEnd - frameStart;

            if (elapsed < FRAME_TIME_MS) {
                timer->DelayMilliseconds(FRAME_TIME_MS - elapsed);
            }

            // 定期的にFPS表示
            if (frameCount % 60 == 0) {
                printf("Frame: %d\n", frameCount);
            }
        }

        printf("\n終了: 合計 %d フレーム実行\n", frameCount);
    }

    void Shutdown() {
        running = false;
    }
};

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    printf("M88 HAL Integration Demo starting...\n");

    // プラットフォームHAL作成（SDL2実装）
    PlatformHAL* platform = CreateSDL2Platform();
    if (!platform) {
        fprintf(stderr, "Failed to create platform HAL\n");
        return 1;
    }

    // プラットフォーム初期化
    if (!platform->Initialize()) {
        fprintf(stderr, "Platform initialization failed\n");
        delete platform;
        return 1;
    }

    // エミュレータコア作成・初期化
    EmulatorCore emulator(platform);
    if (!emulator.Initialize()) {
        fprintf(stderr, "Emulator initialization failed\n");
        platform->Shutdown();
        delete platform;
        return 1;
    }

    // エミュレータ実行
    emulator.Run();

    // クリーンアップ
    emulator.Shutdown();
    platform->Shutdown();
    delete platform;

    printf("Demo finished successfully\n");
    return 0;
}

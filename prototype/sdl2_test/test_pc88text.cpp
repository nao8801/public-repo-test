// PC-8801 Text Screen Test
// Displays PC-88 style boot messages

#include "../../src/sdl2/DrawSDL2.h"
#include "pc88_textscreen.h"
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 200;  // PC-88 text mode
const int BPP = 8;

int main(int argc, char* argv[])
{
    printf("==============================================\n");
    printf("PC-8801 Text Screen Test\n");
    printf("==============================================\n\n");

    // DrawSDL2初期化
    DrawSDL2 draw;
    if (!draw.Init(SCREEN_WIDTH, SCREEN_HEIGHT, BPP)) {
        fprintf(stderr, "DrawSDL2::Init failed\n");
        return 1;
    }

    // PC-8801パレット設定
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

    // テキスクリーン初期化
    PC88TextScreen textscreen;

    // PC-8801風起動メッセージを表示
    textscreen.Print("N88-BASIC(86) Ver 2.1\n", 7);  // 白
    textscreen.Print("Copyright 1986 by NEC Corporation\n", 7);
    textscreen.Print("\n", 7);
    textscreen.Print("How many files(0-15)? ", 6);  // 黄色で質問
    textscreen.SetCursor(23, 3);
    textscreen.Print("3", 7);  // デフォルト値
    textscreen.Print("\n\n", 7);
    textscreen.Print("N88-BASIC(86) Disk BASIC Ver 2.1", 7);
    textscreen.Print(" (C)1986 Hudson soft\n", 7);
    textscreen.Print("Ok\n", 7);

    // カーソル位置に'_'を表示
    int cx, cy;
    textscreen.GetCursor(cx, cy);
    textscreen.PutChar(cx, cy, '_', 7);

    // サンプルプログラムリスト
    textscreen.Print("\n\n", 7);
    textscreen.Print(" 10 SCREEN 0\n", 7);
    textscreen.Print(" 20 CLS\n", 7);
    textscreen.Print(" 30 PRINT \"Hello M88 on SDL2!\"\n", 7);
    textscreen.Print(" 40 FOR I=0 TO 7\n", 7);
    textscreen.Print(" 50   COLOR I\n", 7);
    textscreen.Print(" 60   PRINT \"Color \";I\n", 7);
    textscreen.Print(" 70 NEXT I\n", 7);
    textscreen.Print("\n", 7);
    textscreen.Print("LIST   (Press ESC to exit)\n", 2);  // 赤で説明

    printf("PC-8801 text screen initialized\n");
    printf("Displaying boot message...\n");
    printf("Press ESC to exit\n\n");

    bool running = true;
    int frame = 0;

    while (running) {
        // イベント処理
        if (!draw.ProcessEvents()) {
            running = false;
            break;
        }

        // VRAMロック
        uint8_t* vram;
        int pitch;
        if (!draw.Lock(&vram, &pitch)) {
            fprintf(stderr, "Lock failed\n");
            break;
        }

        // テキスクリーンを描画
        textscreen.Render(vram, SCREEN_WIDTH, SCREEN_HEIGHT, pitch);

        // カーソル点滅（30フレームごと）
        if ((frame / 30) % 2 == 0) {
            textscreen.GetCursor(cx, cy);
            textscreen.PutChar(cx, cy, '_', 7);
        } else {
            textscreen.GetCursor(cx, cy);
            textscreen.PutChar(cx, cy, ' ', 7);
        }

        draw.Unlock();

        // 画面更新
        Draw::Region region;
        region.left = 0;
        region.top = 0;
        region.right = SCREEN_WIDTH;
        region.bottom = SCREEN_HEIGHT;
        draw.DrawScreen(region);

        frame++;

        SDL_Delay(16);  // 約60fps
    }

    draw.Cleanup();

    printf("\nTest finished. Total frames: %d\n", frame);
    return 0;
}

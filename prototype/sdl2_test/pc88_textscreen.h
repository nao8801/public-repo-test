// PC-8801 Text Screen Emulator
// 80x25 text mode with 8x8 font

#pragma once

#include <stdint.h>
#include <string.h>

// PC-8801 text screen specifications
const int TEXT_COLS = 80;
const int TEXT_ROWS = 25;
const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 8;

class PC88TextScreen
{
public:
    PC88TextScreen();
    ~PC88TextScreen();

    // テキストVRAM操作
    void Clear();
    void PutChar(int x, int y, char ch, uint8_t color = 7);
    void PutString(int x, int y, const char* str, uint8_t color = 7);
    void Scroll();

    // フレームバッファへ描画（8bpp indexed color）
    void Render(uint8_t* framebuffer, int width, int height, int pitch);

    // カーソル位置管理
    void SetCursor(int x, int y) { cursor_x = x; cursor_y = y; }
    void GetCursor(int& x, int& y) { x = cursor_x; y = cursor_y; }
    void Print(const char* str, uint8_t color = 7);

private:
    // テキストVRAM（文字コードと色）
    uint8_t text_vram[TEXT_ROWS][TEXT_COLS];
    uint8_t attr_vram[TEXT_ROWS][TEXT_COLS];  // 色属性

    // カーソル位置
    int cursor_x;
    int cursor_y;

    // 8x8フォントデータ取得
    const uint8_t* GetFont(uint8_t ch);
};

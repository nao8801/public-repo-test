// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  SDL2版エントリポイント

#include <SDL2/SDL.h>
#include <stdio.h>
#include "WinCoreSDL2.h"
#include "ConfigSDL2.h"

int main(int argc, char* argv[])
{
    printf("M88 - PC-8801 Emulator (SDL2 version)\n");
    printf("======================================\n\n");

    // コマンドライン引数の解析
    const char* disk_image = nullptr;
    if (argc > 1) {
        disk_image = argv[1];
        printf("Disk image specified: %s\n", disk_image);
    }

    // SDL2初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL2 initialized\n");

    // 設定読み込み（デフォルト値）
    ConfigSDL2 config;
    config.LoadDefaults();

    // コア初期化
    WinCoreSDL2 core;
    if (!core.Init(&config, disk_image)) {
        fprintf(stderr, "WinCore initialization failed\n");
        SDL_Quit();
        return 1;
    }
    printf("WinCore initialized\n");

    // メインループ
    printf("Starting emulation...\n");
    printf("Press ESC to exit\n\n");
    core.Run();

    // 終了処理
    printf("\nShutting down...\n");
    core.Cleanup();
    SDL_Quit();

    printf("M88 terminated successfully\n");
    return 0;
}

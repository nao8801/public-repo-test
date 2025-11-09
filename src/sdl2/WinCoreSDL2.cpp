// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  WinCore SDL2版 実装

#include "WinCoreSDL2.h"
#include <SDL2/SDL.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
//  構築・破棄
//
WinCoreSDL2::WinCoreSDL2()
    : pc88(nullptr)
    , draw(nullptr)
    , diskmgr(nullptr)
    , tapemgr(nullptr)
    , running(false)
{
}

WinCoreSDL2::~WinCoreSDL2()
{
    Cleanup();
}

// ---------------------------------------------------------------------------
//  初期化
//
bool WinCoreSDL2::Init(ConfigSDL2* config)
{
    printf("Initializing WinCoreSDL2...\n");

    // 1. DrawSDL2作成（Init()はPC88::Init()内で呼ばれる）
    printf("  - Creating DrawSDL2...\n");
    draw = new DrawSDL2();
    // 注意: draw->Init()はPC88::Init()内で呼ばれるため、ここでは呼ばない

    // 2. DiskManager初期化
    printf("  - Initializing DiskManager...\n");
    diskmgr = new DiskManager();
    if (!diskmgr->Init()) {
        fprintf(stderr, "DiskManager::Init failed\n");
        delete draw;
        return false;
    }

    // 3. TapeManager作成（PC88::Init内で初期化される）
    printf("  - Creating TapeManager...\n");
    tapemgr = new TapeManager();

    // 4. PC88初期化（この中でdraw->Init()が呼ばれる）
    printf("  - Initializing PC88...\n");
    pc88 = new PC88();
    bool pc88_init_ok = pc88->Init(draw, diskmgr, tapemgr);
    if (!pc88_init_ok) {
        fprintf(stderr, "PC88::Init failed (this may be due to missing ROM files)\n");
        fprintf(stderr, "WARNING: Emulator will not run properly without ROM files\n");
        fprintf(stderr, "Expected ROM files in current directory:\n");
        fprintf(stderr, "  - N88.ROM, N88_0.ROM, N88_1.ROM, N88_2.ROM, N88_3.ROM\n");
        fprintf(stderr, "  - DISK.ROM, N88KNJ1.ROM, N88KNJ2.ROM, etc.\n");
        // ROM読み込みエラーの場合は、エミュレータを起動しない
        delete pc88;
        delete tapemgr;
        delete diskmgr;
        delete draw;
        return false;
    }

    // 5. 設定適用
    printf("  - Applying configuration...\n");
    pc88->ApplyConfig(config->GetPC88Config());

    printf("WinCoreSDL2: Initialization complete\n");
    running = true;
    return true;
}

// ---------------------------------------------------------------------------
//  メインループ
//
void WinCoreSDL2::Run()
{
    uint32_t last_time = SDL_GetTicks();
    const uint32_t frame_time = 1000 / 60;  // 60 FPS = 16.67ms
    uint32_t frame_count = 0;

    printf("Entering main loop (60 FPS target)\n");

    while (running) {
        ProcessEvents();

        // PC88エミュレーション実行
        // 1フレーム = 16.67ms = 1667 ticks (1 tick = 10μs)
        // clock=100 (4MHz), eclock=100 (実効クロック100%)
        pc88->Proceed(1667, 100, 100);

        // 画面更新
        pc88->UpdateScreen(false);

        // フレームレート制御
        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed = current_time - last_time;
        if (elapsed < frame_time) {
            SDL_Delay(frame_time - elapsed);
        }
        last_time = SDL_GetTicks();

        // 1秒ごとにフレームカウント表示
        frame_count++;
        if (frame_count % 60 == 0) {
            printf("Frames: %u (running for %u seconds)\n", frame_count, frame_count / 60);
        }
    }

    printf("Exited main loop after %u frames\n", frame_count);
}

// ---------------------------------------------------------------------------
//  イベント処理
//
void WinCoreSDL2::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            printf("Received SDL_QUIT event\n");
            running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            printf("ESC key pressed\n");
            running = false;
        }
    }
}

// ---------------------------------------------------------------------------
//  終了処理
//
void WinCoreSDL2::Cleanup()
{
    printf("Cleaning up WinCoreSDL2...\n");

    if (pc88) {
        printf("  - Deleting PC88...\n");
        delete pc88;
        pc88 = nullptr;
    }
    if (tapemgr) {
        printf("  - Deleting TapeManager...\n");
        delete tapemgr;
        tapemgr = nullptr;
    }
    if (diskmgr) {
        printf("  - Deleting DiskManager...\n");
        delete diskmgr;
        diskmgr = nullptr;
    }
    if (draw) {
        printf("  - Deleting DrawSDL2...\n");
        delete draw;
        draw = nullptr;
    }

    printf("WinCoreSDL2: Cleanup complete\n");
}

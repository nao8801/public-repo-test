// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  WinCore SDL2版 実装

#include "WinCoreSDL2.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include "pc88/memory.h"
#include "pc88/opnif.h"
#include "pc88/beep.h"
#include "devices/Z80.h"

// ---------------------------------------------------------------------------
//  構築・破棄
//
WinCoreSDL2::WinCoreSDL2()
    : pc88(nullptr)
    , draw(nullptr)
    , diskmgr(nullptr)
    , tapemgr(nullptr)
    , keyboard(nullptr)
    , sound(nullptr)
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
bool WinCoreSDL2::Init(ConfigSDL2* config, const char* disk_image)
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

    // CPU1のダンプを有効化（デバッグ用）
    // 必要に応じてコメントアウトを外して有効化
    // pc88->GetCPU1()->EnableDump(true);

    // 5. KeyboardSDL2初期化と接続
    printf("  - Initializing KeyboardSDL2...\n");
    keyboard = new PC8801::KeyboardSDL2();
    if (!keyboard->Init()) {
        fprintf(stderr, "KeyboardSDL2::Init failed\n");
        delete pc88;
        delete tapemgr;
        delete diskmgr;
        delete draw;
        return false;
    }

    // KeyboardをIOBusに接続
    printf("  - Connecting keyboard to IOBus...\n");
    IOBus* bus = pc88->GetBus1();
    static const IOBus::Connector keyb_connector[] =
    {
        { PC88::pres, IOBus::portout, PC8801::KeyboardSDL2::reset },
        { PC88::vrtc, IOBus::portout, PC8801::KeyboardSDL2::vsync },
        { 0x00, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x01, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x02, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x03, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x04, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x05, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x06, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x07, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x08, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x09, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0a, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0b, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0c, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0d, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0e, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0x0f, IOBus::portin, PC8801::KeyboardSDL2::in },
        { 0, 0, 0 }
    };
    if (!bus->Connect(keyboard, keyb_connector)) {
        fprintf(stderr, "Failed to connect keyboard to IOBus\n");
        delete keyboard;
        delete pc88;
        delete tapemgr;
        delete diskmgr;
        delete draw;
        return false;
    }

    // 6. SoundSDL2初期化と接続
    printf("  - Initializing SoundSDL2...\n");
    sound = new PC8801::SoundSDL2();
    if (!sound->Init(pc88, 44100, 100)) {
        fprintf(stderr, "SoundSDL2::Init failed\n");
        delete sound;
        sound = nullptr;
        // 音声は必須ではないので続行
        fprintf(stderr, "WARNING: Continuing without audio\n");
    }

    // 音源をSoundに接続
    if (sound) {
        printf("  - Connecting sound sources to SoundSDL2...\n");
        if (!pc88->GetOPN1()->Connect(sound)) {
            fprintf(stderr, "WARNING: Failed to connect OPN1 to sound\n");
        }
        if (!pc88->GetOPN2()->Connect(sound)) {
            fprintf(stderr, "WARNING: Failed to connect OPN2 to sound\n");
        }
        if (!pc88->GetBEEP()->Connect(sound)) {
            fprintf(stderr, "WARNING: Failed to connect BEEP to sound\n");
        }
    }

    // 7. 設定適用
    printf("  - Applying configuration...\n");
    pc88->ApplyConfig(config->GetPC88Config());
    keyboard->ApplyConfig(config->GetPC88Config());
    if (sound) {
        sound->ApplyConfig(config->GetPC88Config());
    }

    // 設定適用後にReset()を再度呼ぶ（DIPSWなどの設定を反映するため）
    pc88->Reset();

    // 8. ディスクイメージのマウント（指定されている場合）
    if (disk_image != nullptr) {
        printf("  - Mounting disk image: %s\n", disk_image);
        // Mount(drive, diskname, readonly, index, create)
        // drive=0 (ドライブA), readonly=false, index=0 (最初のディスク), create=false
        if (diskmgr->Mount(0, disk_image, false, 0, false)) {
            printf("  - Disk mounted successfully on drive A:\n");
        } else {
            fprintf(stderr, "WARNING: Failed to mount disk image: %s\n", disk_image);
            fprintf(stderr, "  Emulator will continue without disk.\n");
        }
    }

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

        frame_count++;
    }

}

// ---------------------------------------------------------------------------
//  イベント処理
//
void WinCoreSDL2::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            printf("Received SDL_QUIT event\n");
            running = false;
            break;

        case SDL_KEYDOWN:
            // ESC key exits the emulator
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                printf("ESC key pressed\n");
                running = false;
            }
            // TAB key resets the emulator (useful for disk recognition)
            else if (event.key.keysym.sym == SDLK_TAB) {
                printf("TAB key pressed - Resetting PC88...\n");
                if (pc88) {
                    pc88->Reset();
                    printf("PC88 reset complete\n");
                }
            }
            else if (keyboard) {
                // Forward all other key down events to keyboard emulation
                // Use keycode (logical key) for RDP compatibility
                keyboard->KeyDown(event.key.keysym.sym);
            }
            break;

        case SDL_KEYUP:
            if (keyboard) {
                // Forward key up events to keyboard emulation
                // Use keycode (logical key) for RDP compatibility
                keyboard->KeyUp(event.key.keysym.sym);
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------------
//  終了処理
//
void WinCoreSDL2::Cleanup()
{
    printf("Cleaning up WinCoreSDL2...\n");

    if (keyboard) {
        printf("  - Deleting KeyboardSDL2...\n");
        delete keyboard;
        keyboard = nullptr;
    }
    if (sound) {
        printf("  - Deleting SoundSDL2...\n");
        delete sound;
        sound = nullptr;
    }
    // TapeManagerを先に閉じる（PC88が削除される前にschedulerへの参照を無効化）
    if (tapemgr) {
        printf("  - Closing TapeManager...\n");
        tapemgr->Close();
    }
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

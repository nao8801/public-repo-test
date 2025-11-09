// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  WinCore SDL2版 - コアエミュレーションループ

#ifndef WINCORE_SDL2_H
#define WINCORE_SDL2_H

#include "pc88/pc88.h"
#include "pc88/diskmgr.h"
#include "pc88/tapemgr.h"
#include "DrawSDL2.h"
#include "ConfigSDL2.h"

// ---------------------------------------------------------------------------
//  WinCoreSDL2 - エミュレーションコア
//
class WinCoreSDL2
{
public:
    WinCoreSDL2();
    ~WinCoreSDL2();

    bool Init(ConfigSDL2* config);
    void Run();       // メインループ
    void Cleanup();

private:
    PC88* pc88;
    DrawSDL2* draw;
    DiskManager* diskmgr;
    TapeManager* tapemgr;
    bool running;

    void ProcessEvents();
};

#endif // WINCORE_SDL2_H

// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  ConfigSDL2 - 設定管理

#ifndef CONFIG_SDL2_H
#define CONFIG_SDL2_H

#include "pc88/config.h"

// ---------------------------------------------------------------------------
//  ConfigSDL2 - 設定管理クラス
//  Phase 2ではデフォルト値を使用、将来的にINIファイル読み込みを追加予定
//
class ConfigSDL2
{
public:
    ConfigSDL2();
    ~ConfigSDL2();

    void LoadDefaults();  // デフォルト値設定

    PC8801::Config* GetPC88Config() { return &pc88config; }

private:
    PC8801::Config pc88config;
};

#endif // CONFIG_SDL2_H

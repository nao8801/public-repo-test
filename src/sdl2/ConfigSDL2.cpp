// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  ConfigSDL2 実装

#include "ConfigSDL2.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  構築・破棄
//
ConfigSDL2::ConfigSDL2()
{
    memset(&pc88config, 0, sizeof(pc88config));
    LoadDefaults();
}

ConfigSDL2::~ConfigSDL2()
{
}

// ---------------------------------------------------------------------------
//  デフォルト値設定
//  PC-8801mk2SR の標準的な設定
//
void ConfigSDL2::LoadDefaults()
{
    printf("Loading default PC-8801mk2SR configuration...\n");

    // 基本設定
    pc88config.basicmode = PC8801::Config::N88V2;  // N88-BASIC V2モード

    // フラグ設定
    pc88config.flags = PC8801::Config::enableopna;     // OPNA有効
    pc88config.flag2 = 0;

    // CPU設定
    pc88config.clock = 100;          // 4MHz (100 = 1単位100kHz)
    pc88config.speed = 100;          // 100%速度
    pc88config.mainsubratio = 16;    // メイン:サブ CPU比 16:1
    pc88config.cpumode = PC8801::Config::msauto;

    // サウンド設定
    pc88config.soundbuffer = 100;    // 100ms
    pc88config.opnclock = 8;         // OPNクロック (8MHz)
    // Win32版のデフォルト値に合わせる
    // Win32版: FM=0 (VOLUME_BIAS), SSG=97 (またはリセット時は-3)
    // 音量範囲は -40 ～ +20 dB
    pc88config.volfm = 0;            // FM音源音量 (Win32版デフォルト: 0)
    pc88config.volssg = -3;          // SSG音量 (Win32版リセット時デフォルト: -3)
    pc88config.voladpcm = 80;        // ADPCM音量
    pc88config.volrhythm = 80;       // リズム音量
    pc88config.volbd = 80;           // バスドラム
    pc88config.volsd = 80;           // スネアドラム
    pc88config.voltop = 80;          // トップシンバル
    pc88config.volhh = 80;           // ハイハット
    pc88config.voltom = 80;          // タム
    pc88config.volrim = 80;          // リムショット

    // メモリ設定
    pc88config.erambanks = 4;        // 拡張RAM 4バンク = 128KB

    // その他
    pc88config.refreshtiming = 0;
    pc88config.dipsw = 1829;  // Win32版と同じデフォルト値（DIPSW設定）
    pc88config.keytype = PC8801::Config::AT106;
    pc88config.mousesensibility = 50;
    pc88config.lpffc = 8000;         // LPF カットオフ周波数
    pc88config.lpforder = 2;
    pc88config.romeolatency = 0;
    pc88config.winposx = -1;
    pc88config.winposy = -1;

    printf("  - BASIC Mode: N88-BASIC V2\n");
    printf("  - CPU Clock: 4MHz (100%%)\n");
    printf("  - Main:Sub CPU Ratio: 16:1\n");
    printf("  - Extended RAM: 128KB (4 banks)\n");
    printf("  - OPNA: Enabled\n");
    printf("Configuration loaded successfully\n");
}

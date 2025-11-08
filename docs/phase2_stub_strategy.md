# Phase 2: スタブ化戦略 - 最小限ビルドの実現

## 🎯 目標

「黒い画面が表示される」レベルの実行可能ファイル `m88` をビルドする。

---

## 📦 最小限の構成

### ビルドターゲット

```
m88core.a (既存) + sdl2_minimal.a (新規) → m88 (実行ファイル)
```

---

## 🗂️ ファイル構成

### A. 既存のコアライブラリ (m88core.a) - 変更なし

```
src/common/   (9ファイル)  ✅ ビルド済み
src/devices/  (10ファイル) ✅ ビルド済み
src/pc88/     (24ファイル) ✅ ビルド済み
```

**追加**: zlibサポート
```cmake
# システムzlibを使用
find_package(ZLIB REQUIRED)
target_link_libraries(m88core PRIVATE ZLIB::ZLIB)
```

---

### B. SDL2最小実装層 (sdl2_minimal.a) - 新規作成

#### 📁 src/sdl2/ ディレクトリ構成

```
src/sdl2/
├── DrawSDL2.cpp/h          ✅ 既存 (描画)
├── main_sdl2.cpp           🆕 新規 (エントリポイント)
├── WinCoreSDL2.cpp/h       🆕 新規 (コアループ)
├── ConfigSDL2.cpp/h        🆕 新規 (設定管理)
├── FileSDL2.cpp/h          🆕 新規 (ファイルI/O)
└── stubs/                  🆕 新規 (スタブ実装)
    ├── ui_stub.cpp
    ├── status_stub.cpp
    ├── module_stub.cpp
    ├── extdev_stub.cpp
    └── misc_stub.cpp
```

---

## 🔨 新規ファイルの実装内容

### 1. main_sdl2.cpp - エントリポイント

```cpp
// src/sdl2/main_sdl2.cpp
#include <SDL2/SDL.h>
#include "WinCoreSDL2.h"
#include "ConfigSDL2.h"

int main(int argc, char* argv[])
{
    // SDL2初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // 設定読み込み（デフォルト値）
    ConfigSDL2 config;
    config.LoadDefaults();

    // コア初期化
    WinCoreSDL2 core;
    if (!core.Init(&config)) {
        fprintf(stderr, "WinCore initialization failed\n");
        SDL_Quit();
        return 1;
    }

    // メインループ
    core.Run();

    // 終了処理
    core.Cleanup();
    SDL_Quit();

    return 0;
}
```

**依存関係**: SDL2, WinCoreSDL2, ConfigSDL2

---

### 2. WinCoreSDL2.cpp/h - コアループ（wincore.cppの簡略版）

```cpp
// src/sdl2/WinCoreSDL2.h
#ifndef WINCORE_SDL2_H
#define WINCORE_SDL2_H

#include "pc88/pc88.h"
#include "pc88/diskmgr.h"
#include "pc88/tapemgr.h"
#include "DrawSDL2.h"
#include "ConfigSDL2.h"

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

#endif
```

```cpp
// src/sdl2/WinCoreSDL2.cpp
#include "WinCoreSDL2.h"
#include <SDL2/SDL.h>

WinCoreSDL2::WinCoreSDL2()
    : pc88(nullptr), draw(nullptr), diskmgr(nullptr), tapemgr(nullptr), running(false)
{
}

WinCoreSDL2::~WinCoreSDL2()
{
    Cleanup();
}

bool WinCoreSDL2::Init(ConfigSDL2* config)
{
    // 1. DrawSDL2初期化
    draw = new DrawSDL2();
    if (!draw->Init(640, 400, 8)) {
        fprintf(stderr, "DrawSDL2::Init failed\n");
        return false;
    }

    // 2. DiskManager初期化
    diskmgr = new DiskManager();
    if (!diskmgr->Init()) {
        fprintf(stderr, "DiskManager::Init failed\n");
        delete draw;
        return false;
    }

    // 3. TapeManager作成（PC88::Init内で初期化される）
    tapemgr = new TapeManager();

    // 4. PC88初期化
    pc88 = new PC88();
    if (!pc88->Init(draw, diskmgr, tapemgr)) {
        fprintf(stderr, "PC88::Init failed\n");
        delete tapemgr;
        delete diskmgr;
        delete draw;
        return false;
    }

    // 5. 設定適用
    pc88->ApplyConfig(config->GetPC88Config());

    printf("WinCoreSDL2: Initialization complete\n");
    running = true;
    return true;
}

void WinCoreSDL2::Run()
{
    uint32_t last_time = SDL_GetTicks();
    const uint32_t frame_time = 1000 / 60;  // 60 FPS = 16.67ms

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
    }
}

void WinCoreSDL2::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
        }
    }
}

void WinCoreSDL2::Cleanup()
{
    if (pc88) {
        delete pc88;
        pc88 = nullptr;
    }
    if (tapemgr) {
        delete tapemgr;
        tapemgr = nullptr;
    }
    if (diskmgr) {
        delete diskmgr;
        diskmgr = nullptr;
    }
    if (draw) {
        delete draw;
        draw = nullptr;
    }
}
```

**依存関係**: SDL2, DrawSDL2, PC88, DiskManager, TapeManager

---

### 3. ConfigSDL2.cpp/h - 設定管理（wincfg.cppの簡略版）

```cpp
// src/sdl2/ConfigSDL2.h
#ifndef CONFIG_SDL2_H
#define CONFIG_SDL2_H

#include "pc88/config.h"

class ConfigSDL2
{
public:
    ConfigSDL2();
    ~ConfigSDL2();

    void LoadDefaults();  // デフォルト値設定
    // TODO: 将来的にINIファイル読み込み追加

    PC8801::Config* GetPC88Config() { return &pc88config; }

private:
    PC8801::Config pc88config;
};

#endif
```

```cpp
// src/sdl2/ConfigSDL2.cpp
#include "ConfigSDL2.h"

ConfigSDL2::ConfigSDL2()
{
    LoadDefaults();
}

ConfigSDL2::~ConfigSDL2()
{
}

void ConfigSDL2::LoadDefaults()
{
    // PC-8801mk2SR デフォルト設定
    pc88config.basicmode = PC8801::Config::N88V2;  // N88-BASIC V2モード

    // フラグ設定
    pc88config.flags = PC8801::Config::enableopna      // OPNA有効
                     | PC8801::Config::showstatusbar;  // ステータスバー表示
    pc88config.flag2 = 0;

    // CPU設定
    pc88config.clock = 100;          // 4MHz (100 = 1単位100kHz)
    pc88config.speed = 100;          // 100%速度
    pc88config.mainsubratio = 16;    // メイン:サブ CPU比 16:1
    pc88config.cpumode = PC8801::Config::msauto;

    // サウンド設定
    pc88config.soundbuffer = 100;    // 100ms
    pc88config.opnclock = 8;         // OPNクロック (8MHz)
    pc88config.volfm = 80;           // FM音源音量 (0-128)
    pc88config.volssg = 80;          // SSG音量
    pc88config.voladpcm = 80;        // ADPCM音量
    pc88config.volrhythm = 80;       // リズム音量

    // メモリ設定
    pc88config.erambanks = 4;        // 拡張RAM 4バンク = 128KB

    // その他
    pc88config.refreshtiming = 0;
    pc88config.dipsw = 0;
    pc88config.keytype = PC8801::Config::AT106;

    printf("ConfigSDL2: Loaded default PC-8801mk2SR configuration\n");
}
```

**依存関係**: pc88/config.h

---

### 4. FileSDL2.cpp/h - ファイルI/O（file.cppの標準C++化）

```cpp
// src/sdl2/FileSDL2.h
#ifndef FILE_SDL2_H
#define FILE_SDL2_H

#include <string>
#include <fstream>

class FileSDL2
{
public:
    static bool ReadFile(const char* path, uint8_t** buffer, size_t* size);
    static bool WriteFile(const char* path, const uint8_t* buffer, size_t size);
    static bool FileExists(const char* path);
    static std::string GetExecutablePath();
};

#endif
```

```cpp
// src/sdl2/FileSDL2.cpp
#include "FileSDL2.h"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

bool FileSDL2::ReadFile(const char* path, uint8_t** buffer, size_t* size)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    *size = file.tellg();
    file.seekg(0, std::ios::beg);

    *buffer = new uint8_t[*size];
    file.read((char*)*buffer, *size);
    file.close();

    return true;
}

bool FileSDL2::WriteFile(const char* path, const uint8_t* buffer, size_t size)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write((const char*)buffer, size);
    file.close();

    return true;
}

bool FileSDL2::FileExists(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

std::string FileSDL2::GetExecutablePath()
{
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(dirname(buf));
    }
    return ".";
}
```

**依存関係**: C++標準ライブラリ、POSIX API

---

### 5. stubs/ - スタブ実装

#### ui_stub.cpp
```cpp
// src/sdl2/stubs/ui_stub.cpp
// UI機能のスタブ（ダイアログなどは全て無効化）

void ShowAboutDialog() { /* do nothing */ }
void ShowConfigDialog() { /* do nothing */ }
void UpdateStatusBar(const char* msg) { printf("Status: %s\n", msg); }
```

#### status_stub.cpp
```cpp
// src/sdl2/stubs/status_stub.cpp
// ステータス表示のスタブ

void Status_Update(int type, const char* msg) {
    printf("[Status %d] %s\n", type, msg);
}
```

#### module_stub.cpp
```cpp
// src/sdl2/stubs/module_stub.cpp
// プラグインモジュールのスタブ（プラグイン無効化）

bool LoadModule(const char* path) { return false; }
void UnloadAllModules() { /* do nothing */ }
```

#### extdev_stub.cpp
```cpp
// src/sdl2/stubs/extdev_stub.cpp
// 外部デバイスのスタブ

bool InitExternalDevices() { return true; }
void CleanupExternalDevices() { /* do nothing */ }
```

#### misc_stub.cpp
```cpp
// src/sdl2/stubs/misc_stub.cpp
// その他のユーティリティスタブ

void Log(const char* format, ...) {
    // 標準出力にリダイレクト
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
```

---

## 📋 CMakeLists.txt の更新

```cmake
# src/sdl2/CMakeLists.txt に追加

set(SDL2_MINIMAL_SOURCES
    main_sdl2.cpp
    WinCoreSDL2.cpp
    ConfigSDL2.cpp
    FileSDL2.cpp
    DrawSDL2.cpp  # 既存
    stubs/ui_stub.cpp
    stubs/status_stub.cpp
    stubs/module_stub.cpp
    stubs/extdev_stub.cpp
    stubs/misc_stub.cpp
)

add_library(sdl2_minimal STATIC ${SDL2_MINIMAL_SOURCES})
target_link_libraries(sdl2_minimal PUBLIC SDL2)

# 実行ファイル
add_executable(m88 main_sdl2.cpp)
target_link_libraries(m88
    sdl2_minimal
    m88core
    SDL2
    ZLIB::ZLIB
    pthread
    m
)
```

---

## 🚀 ビルド手順

```bash
cd /home/user/public-repo-test
mkdir -p build
cd build
cmake ..
make m88
```

**期待される結果**:
```
[ 50%] Built target m88core
[ 75%] Built target sdl2_minimal
[100%] Built target m88
```

---

## ✅ 成功の定義

実行時：
```bash
./m88
```

**期待される動作**:
1. ウィンドウが開く（640x400、黒い画面）
2. エラーメッセージなし
3. ESCキーで終了できる

**この時点では**:
- ROM読み込みなし（PC88未初期化でOK）
- 描画なし（黒い画面でOK）
- 音なし（Audio初期化のみでOK）

---

## 📝 次のステップ (Phase 3)

1. ROM読み込み対応
2. PC88::Init() の実装
3. Screen.cpp連携
4. キーボード入力
5. ディスクイメージ読み込み

---

作成日: 2025-11-08

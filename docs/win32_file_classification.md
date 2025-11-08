# Win32ファイル分類 - SDL2移植のための優先度分析

## 📋 概要

M88のWin32層（46ファイル）を、Ubuntu版実装における優先度で分類しました。

---

## 🎯 優先度レベル

### ⭐⭐⭐ **Priority 1: 必須（コア）** - 最初にスタブ実装が必要

| ファイル | 役割 | SDL2化の方針 |
|---------|------|------------|
| **main.cpp** | エントリポイント (WinMain) | → SDL2版main()に置き換え |
| **wincore.cpp** | エミュレータコアループ | → PC88::Init/Run呼び出し維持、Windows API除去 |
| **wincfg.cpp** | 設定管理 (INI読み込み) | → デフォルト値固定、または簡易INI読み込み |
| **file.cpp** | ファイルI/O (ROM/ディスク) | → 標準C++ fstream化 |
| **windraw.cpp** | 描画管理 (WinDrawSub管理) | → DrawSDL2に統合 |
| **winsound.cpp** | サウンド管理 | → SDL2 Audio APIに置き換え |

**合計**: 6ファイル

---

### ⭐⭐ **Priority 2: 描画・サウンド・入力** - SDL2化対象

#### 描画系
| ファイル | 役割 | 対応方針 |
|---------|------|---------|
| DrawDDS.cpp | DirectDraw Surface実装 | → DrawSDL2で代替 |
| DrawGDI.cpp | GDI実装 | → DrawSDL2で代替 |
| DrawD2D.cpp | Direct2D実装 | → DrawSDL2で代替 |
| drawddw.cpp | DirectDraw Window? | → DrawSDL2で代替 |
| dderr.cpp | DirectDrawエラー処理 | → 不要 (SDL2はエラー処理統一) |

#### サウンド系
| ファイル | 役割 | 対応方針 |
|---------|------|---------|
| soundds.cpp | DirectSound実装 | → SDL2 Audioで代替 |
| soundds2.cpp | DirectSound2実装 | → SDL2 Audioで代替 |
| soundwo.cpp | WaveOut実装 | → SDL2 Audioで代替 |

#### 入力系
| ファイル | 役割 | 対応方針 |
|---------|------|---------|
| WinKeyIF.cpp | キーボードインターフェース | → SDL2 Keyboard APIで代替 |
| WinJoy.cpp | ジョイスティック | → SDL2 Gamepad APIで代替 |
| winmouse.cpp | マウス | → SDL2 Mouse APIで代替 |
| keybconn.cpp | キーボード接続管理 | → SDL2で再実装 |

**合計**: 13ファイル → **すべてDrawSDL2/SDL2で代替、ビルドから除外**

---

### ⭐ **Priority 3: システム・ユーティリティ** - スタブまたは簡略化

| ファイル | 役割 | 対応方針 |
|---------|------|---------|
| guid.cpp | GUID定義 | → そのままビルド (Windows APIなし) |
| module.cpp | モジュール管理 (プラグイン) | → スタブ実装 (プラグイン無効化) |
| sequence.cpp | シーケンス管理 | → 要調査、必要なら簡略実装 |
| timekeep.cpp | タイムキーピング | → std::chrono化 |
| winvars.cpp | グローバル変数定義 | → そのままビルド |
| winexapi.cpp | 外部API | → スタブ実装 |
| extdev.cpp | 外部デバイス | → スタブ実装 (後回し) |
| diag.cpp | 診断・ログ出力 | → printfまたはstdout化 |
| pch.cpp | プリコンパイルヘッダ | → 不要 (CMakeではPCH別管理) |

**合計**: 9ファイル

---

### ❌ **Priority 4: UI・ダイアログ** - 後回し（スタブ化）

| ファイル | 役割 | 対応方針 |
|---------|------|---------|
| ui.cpp | UI管理 (メニュー、ダイアログ) | → スタブ実装 |
| 88config.cpp | 設定ダイアログ | → 後回し (wincfgでデフォルト値使用) |
| about.cpp | Aboutダイアログ | → 不要 |
| cfgpage.cpp | 設定ページ | → 不要 |
| status.cpp | ステータス表示 | → SDL2でテキスト描画、または省略 |
| newdisk.cpp | 新規ディスク作成ダイアログ | → 後回し |

**合計**: 6ファイル

---

### ❌ **Priority 5: デバッグ・モニタ系** - 除外（後回し）

| ファイル | 役割 | 備考 |
|---------|------|------|
| basmon.cpp | BASICモニタ | Debug機能、後回し |
| codemon.cpp | コードモニタ | Debug機能、後回し |
| iomon.cpp | I/Oモニタ | Debug機能、後回し |
| loadmon.cpp | ロードモニタ | Debug機能、後回し |
| memmon.cpp | メモリモニタ | Debug機能、後回し |
| mvmon.cpp | メモリビューモニタ | Debug機能、後回し |
| regmon.cpp | レジスタモニタ | Debug機能、後回し |
| soundmon.cpp | サウンドモニタ | Debug機能、後回し |
| winmon.cpp | モニタ統合 | Debug機能、後回し |

**合計**: 9ファイル → **ビルドから完全除外**

---

### ❌ **Priority 6: 実機接続・その他** - 除外

| ファイル | 役割 | 備考 |
|---------|------|------|
| romeo/piccolo.cpp | 実機接続 (Romeo) | プラグイン、優先度ゼロ |
| romeo/piccolo_gimic.cpp | GIMIC接続 | プラグイン、優先度ゼロ |
| romeo/piccolo_romeo.cpp | Romeo接続 | プラグイン、優先度ゼロ |
| filetest.cpp | ファイルテスト | テストコード、不要 |
| instthnk.cpp | インストールサンク? | 用途不明、後回し |

**合計**: 5ファイル → **ビルドから完全除外**

---

## 📊 サマリー

| カテゴリ | ファイル数 | 対応方針 |
|---------|----------|---------|
| ✅ Priority 1 (必須コア) | 6 | スタブ実装・SDL2化 |
| ✅ Priority 2 (描画/音/入力) | 13 | SDL2で代替、除外 |
| ⚠️ Priority 3 (システム) | 9 | スタブまたは簡略化 |
| ⏸️ Priority 4 (UI/ダイアログ) | 6 | スタブ化、後回し |
| ❌ Priority 5 (Debug/Monitor) | 9 | 完全除外 |
| ❌ Priority 6 (実機接続/その他) | 5 | 完全除外 |
| **合計** | **48** | - |

---

## 🎯 Phase 2の最小実装ターゲット

### ステップ1: ビルド可能な状態にする

**含めるファイル** (15ファイル):
```
Priority 1: 6ファイル (main, wincore, wincfg, file, windraw, winsound)
Priority 3: 9ファイル (guid, module, sequence, timekeep, winvars, winexapi, extdev, diag, pch)
```

**除外するファイル** (33ファイル):
```
Priority 2: 13ファイル (すべてSDL2で代替)
Priority 4: 6ファイル (UI/ダイアログ)
Priority 5: 9ファイル (mon系)
Priority 6: 5ファイル (Romeo/その他)
```

### ステップ2: スタブ実装の内容

#### main.cpp → src/sdl2/main_sdl2.cpp
```cpp
#include <SDL2/SDL.h>
#include "pc88/pc88.h"
#include "DrawSDL2.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // PC88初期化
    // WinCoreの機能を簡略化して実装

    // メインループ
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // PC88::Run() 相当の処理

        SDL_Delay(16); // 60 FPS
    }

    SDL_Quit();
    return 0;
}
```

#### wincfg.cpp のスタブ化
- すべての設定値をデフォルト定数で返す
- INI読み込みは無効化（または簡易実装）

#### file.cpp の標準C++化
- Windows API (CreateFile, ReadFile) → std::ifstream/ofstream
- MAX_PATH → PATH_MAX

---

## 次のアクション

1. **CMakeLists.txt更新** - Priority 1+3のみビルド対象に
2. **スタブファイル作成** - src/sdl2/main_sdl2.cpp, stubs/wincfg_stub.cpp など
3. **ビルド試行** - エラーを1つずつ潰す
4. **リンク成功** - 実行ファイル m88 生成
5. **起動確認** - 「黒い画面」が表示されればOK！

---

作成日: 2025-11-08

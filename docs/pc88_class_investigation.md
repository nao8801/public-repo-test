# PC88クラスの調査結果 - Phase 2実装のために

## 📋 概要

Phase 2実装（WinCoreSDL2.cpp）のために、PC88クラスの初期化・実行・描画の仕組みを調査しました。

---

## 🔍 調査結果

### 1. PC88クラスの基本構造

**ヘッダー**: `src/pc88/pc88.h`

**継承関係**:
```cpp
class PC88 : public Scheduler, public ICPUTime
```

**主要メソッド**:
```cpp
bool Init(Draw* draw, DiskManager* diskmgr, TapeManager* tape);
void DeInit();
void Reset();
int Proceed(uint us, uint clock, uint eff);
void ApplyConfig(PC8801::Config*);
void UpdateScreen(bool refresh = false);
```

---

### 2. PC88::Init() の仕様

**シグネチャ**:
```cpp
bool Init(Draw* draw, DiskManager* diskmgr, TapeManager* tape);
```

**引数**:

| パラメータ | 型 | 説明 | Phase 2での対応 |
|-----------|---|------|----------------|
| **draw** | Draw* | 描画インターフェース | → DrawSDL2を渡す ✅ |
| **diskmgr** | DiskManager* | ディスク管理 | → 新規作成が必要 ⚠️ |
| **tape** | TapeManager* | テープ管理 | → 新規作成が必要 ⚠️ |

**実装の流れ** (`src/pc88/pc88.cpp:79-114`):

```cpp
bool PC88::Init(Draw* _draw, DiskManager* disk, TapeManager* tape)
{
    draw = _draw;
    diskmgr = disk;
    tapemgr = tape;

    // 1. Scheduler初期化
    if (!Scheduler::Init())
        return false;

    // 2. Draw初期化 (640x400, 8bpp)
    if (!draw->Init(640, 400, 8))
        return false;

    // 3. TapeManager初期化
    if (!tapemgr->Init(this, 0, 0))  // this=Scheduler*, nullptr, 0
        return false;

    // 4. CPU1/CPU2のメモリページ初期化
    MemoryPage* read, * write;
    cpu1.GetPages(&read, &write);
    if (!mm1.Init(0x10000, read, write))
        return false;

    cpu2.GetPages(&read, &write);
    if (!mm2.Init(0x10000, read, write))
        return false;

    // 5. IOバス初期化
    if (!bus1.Init(portend, &devlist) || !bus2.Init(portend2, &devlist))
        return false;

    // 6. デバイス接続 (重要！)
    if (!ConnectDevices() || !ConnectDevices2())
        return false;

    // 7. リセット
    Reset();
    region.Reset();
    clock = 1;
    return true;
}
```

**ConnectDevices()の役割**:
- Base, DMAC, CRTC, Memory, Screen, INTC, FDC, SubSystem, OPNIF, Beep, JoyPad などを作成
- 各デバイスをIOBusに接続
- 約300行の大きな関数（`pc88.cpp:290-`）

---

### 3. DiskManager と TapeManager

#### DiskManager (`src/pc88/diskmgr.h`)

```cpp
class DiskManager
{
public:
    DiskManager();
    ~DiskManager();
    bool Init();  // パラメータなし！

    // ディスクマウント（後で必要）
    bool Mount(uint drive, const char* diskname, bool readonly, int index, bool create);
    bool Unmount(uint drive);

    PC8801::FDU* GetFDU(int dr);  // FDC用のインターフェース
};
```

**Phase 2での対応**:
```cpp
DiskManager* diskmgr = new DiskManager();
if (!diskmgr->Init()) {
    // エラー処理
}
// PC88::Init()に渡す
```

#### TapeManager (`src/pc88/tapemgr.h`)

```cpp
class TapeManager : public Device
{
public:
    TapeManager();
    ~TapeManager();
    bool Init(Scheduler* s, IOBus* bus, int pin);

    bool Open(const char* file);
    bool Close();
};
```

**PC88::Init()での呼び出し**:
```cpp
if (!tapemgr->Init(this, 0, 0))  // Scheduler*, nullptr, 0
    return false;
```

**Phase 2での対応**:
```cpp
TapeManager* tapemgr = new TapeManager();
// Init()はPC88::Init()の中で呼ばれる
```

---

### 4. PC88::UpdateScreen() の仕組み

**実装** (`src/pc88/pc88.cpp:184-229`):

```cpp
void PC88::UpdateScreen(bool refresh)
{
    int dstat = draw->GetStatus();
    if (dstat & Draw::shouldrefresh)
        refresh = true;

    if (!updated || refresh)
    {
        if (!(cfgflags & Config::drawprioritylow) ||
            (dstat & (Draw::readytodraw | Draw::shouldrefresh)))
        {
            int bpl;
            uint8* image;

            // 1. VRAMロック
            if (draw->Lock(&image, &bpl))
            {
                // 2. CRTC が画面を更新
                crtc->UpdateScreen(image, bpl, region, refresh);

                // 3. Screen が画面を更新
                scrn->UpdateScreen(image, bpl, region, refresh);

                // 4. パレット更新
                bool palchanged = scrn->UpdatePalette(draw);

                draw->Unlock();
                updated = palchanged || region.Valid();
            }
        }
    }

    // 5. 描画実行
    if (draw->GetStatus() & Draw::readytodraw)
    {
        if (updated)
        {
            updated = false;
            draw->DrawScreen(region);
            region.Reset();
        }
        else
        {
            Draw::Region r;
            r.Reset();
            draw->DrawScreen(r);
        }
    }
}
```

**重要なポイント**:
- `crtc->UpdateScreen()` と `scrn->UpdateScreen()` が実際の描画処理
- `draw->Lock() / Unlock()` で VRAMバッファを取得
- `draw->DrawScreen()` で画面に転送

---

### 5. PC88::Proceed() - メインループ

**実装** (`src/pc88/pc88.cpp:127-132`):

```cpp
int PC88::Proceed(uint ticks, uint clk, uint ecl)
{
    clock = Max(1, clk);
    eclock = Max(1, ecl);
    return Scheduler::Proceed(ticks);
}
```

**パラメータ**:
- `ticks`: 1 tick = 10μs
- `clk`: CPUクロック（通常は100 = 1MHz単位）
- `ecl`: 実効クロック

**Scheduler::Proceed()の中で**:
- `Execute()` が呼ばれる → Z80エミュレーション実行
- タイマーイベント処理
- VSync処理

---

### 6. PC8801::Config のデフォルト値

**ヘッダー**: `src/pc88/config.h`

**主要な設定項目**:

```cpp
class Config
{
public:
    enum BASICMode
    {
        N80 = 0x00,      // PC-8001モード
        N80V2 = 0x12,    // N80 V2モード
        N88V2 = 0x31,    // N88 V2モード（標準）
        N88V2CD = 0x71,  // N88 V2 + CD-ROM
    };

    int flags;           // Flags enum のビットマスク
    int flag2;           // Flag2 enum のビットマスク
    int clock;           // CPUクロック（通常100 = 4MHz）
    int speed;           // 実行速度
    int mainsubratio;    // メイン:サブCPU比（通常16）
    int soundbuffer;     // サウンドバッファサイズ（ms）

    int volfm, volssg, voladpcm, volrhythm;  // 音量設定

    BASICMode basicmode; // 動作モード
};
```

**推奨デフォルト値** (Phase 2用):

```cpp
Config config;
config.basicmode = Config::N88V2;        // N88-BASIC V2モード
config.flags = Config::enableopna        // OPNA有効
             | Config::showstatusbar;    // ステータスバー
config.flag2 = 0;
config.clock = 100;                      // 4MHz
config.speed = 100;                      // 100%
config.mainsubratio = 16;                // メイン:サブ = 16:1
config.soundbuffer = 100;                // 100ms

// 音量（0-128）
config.volfm = 80;
config.volssg = 80;
config.voladpcm = 80;
config.volrhythm = 80;
```

---

## 🎯 Phase 2実装への適用

### WinCoreSDL2::Init() の実装方針（改訂版）

```cpp
bool WinCoreSDL2::Init(ConfigSDL2* config)
{
    // 1. DrawSDL2初期化
    draw = new DrawSDL2();
    if (!draw->Init(640, 400, 8)) {
        return false;
    }

    // 2. DiskManager初期化
    diskmgr = new DiskManager();
    if (!diskmgr->Init()) {
        delete draw;
        return false;
    }

    // 3. TapeManager初期化（PC88::Init内で初期化される）
    tapemgr = new TapeManager();

    // 4. PC88初期化
    pc88 = new PC88();
    if (!pc88->Init(draw, diskmgr, tapemgr)) {
        delete tapemgr;
        delete diskmgr;
        delete draw;
        return false;
    }

    // 5. 設定適用
    pc88->ApplyConfig(config->GetPC88Config());

    running = true;
    return true;
}
```

### WinCoreSDL2::Run() の実装方針

```cpp
void WinCoreSDL2::Run()
{
    uint32_t last_time = SDL_GetTicks();
    const uint32_t frame_time = 1000 / 60;  // 60 FPS

    while (running) {
        ProcessEvents();

        // PC88エミュレーション実行
        // Proceed(ticks, clock, eclock)
        // ticks = 1tick = 10μs, 1フレーム = 16.67ms = 1667 ticks
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
```

---

## 📝 Phase 2実装で必要な追加ファイル

### 新規追加が必要なクラス

1. **DiskManager** - 既存クラス、ヘッダーインクルードのみ
2. **TapeManager** - 既存クラス、ヘッダーインクルードのみ

### WinCoreSDL2 のメンバー変数（改訂）

```cpp
class WinCoreSDL2
{
private:
    PC88* pc88;
    DrawSDL2* draw;
    DiskManager* diskmgr;   // 追加
    TapeManager* tapemgr;   // 追加
    bool running;
};
```

---

## ⚠️ 注意点

### 1. ROM ファイルの読み込み

PC88::Init()の中の`ConnectDevices()`で、以下のROMが必要になります：

- **N88.ROM** - N88-BASIC ROM
- **N80.ROM** - N80-BASIC ROM (オプション)
- **DISK.ROM** - Disk BASIC ROM
- **KANJI1.ROM** - 漢字ROM 第1水準
- **KANJI2.ROM** - 漢字ROM 第2水準 (オプション)

**Phase 2では**:
- ROM読み込み失敗を許容する（エラーでも続行）
- または、ダミーROMを用意する

### 2. Screen/CRTC の依存関係

`pc88->UpdateScreen()`は、以下のクラスが正しく初期化されている必要があります：

- **Screen** - 画面描画管理
- **CRTC** - CRTコントローラ
- **Memory** - VRAMアクセス

これらは`ConnectDevices()`内で自動的に作成されます。

### 3. サウンドの初期化

OPNAやBeepは`ConnectDevices()`で作成されますが、サウンド出力には別途`SoundBuffer`の初期化が必要です。

**Phase 2では**:
- サウンド出力はスキップ（無音でOK）
- SDL2 Audioは後回し

---

## 🚀 次のステップ

### Phase 2-A: 最小限の動作（黒い画面）

1. ✅ DrawSDL2初期化
2. ✅ DiskManager/TapeManager作成
3. ✅ PC88::Init()呼び出し
4. ⚠️ ROM読み込み失敗を無視
5. ✅ Proceed()で1フレーム実行
6. ✅ UpdateScreen()呼び出し

→ **黒い画面が表示される** ✨

### Phase 2-B: ROM読み込み対応

1. FileSDL2でROMファイル読み込み
2. Memory::Init()にROMデータを渡す
3. 実際のBASIC画面が表示される

### Phase 2-C: キーボード入力

1. SDL2キーイベント取得
2. PC-8801キーコードへ変換
3. IOBusへ送信

---

## 📊 サマリー

| 項目 | 状態 | 備考 |
|-----|------|------|
| PC88::Init()の仕様確認 | ✅ 完了 | Draw, DiskManager, TapeManager必要 |
| DiskManager/TapeManager調査 | ✅ 完了 | Init()は簡単 |
| UpdateScreen()の仕組み | ✅ 完了 | Lock/Unlock, DrawScreen |
| Proceed()の使い方 | ✅ 完了 | 1フレーム=1667ticks |
| Config設定値 | ✅ 完了 | デフォルト値確定 |
| ROM読み込み | ⚠️ 課題 | Phase 2-Bで対応 |

---

作成日: 2025-11-08

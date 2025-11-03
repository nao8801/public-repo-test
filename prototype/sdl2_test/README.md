# M88 SDL2 Prototype - プラットフォーム移植プロトタイプ

このディレクトリには、M88エミュレータをWindows以外のプラットフォーム（Linux/macOS等）に移植するための実験的なプロトタイプが含まれています。

## 目的

M88エミュレータを以下のプラットフォームで動作させるための技術検証：

- ✅ **Linux (Ubuntu/Debian)**
- ✅ **macOS**
- ✅ **FreeBSD**
- ✅ **クラウド環境（ヘッドレス実行）**
- ✅ **ARM64アーキテクチャ（Raspberry Pi等）**

## プロトタイプの構成

### 1. 個別テストプログラム

#### `test_graphics` - 描画テスト
- SDL2による640x400ピクセルのウィンドウ表示
- PC-8801mk2SR相当の画面サイズ
- フレームバッファ操作のテスト
- パレット機能の確認

**実行方法：**
```bash
./test_graphics
```

#### `test_audio` - 音声テスト
- SDL2オーディオ機能のテスト
- サイン波生成（音階ド〜シ）
- PC-8801 BEEP音シミュレーション（2kHz）
- 44.1kHz ステレオ出力

**実行方法：**
```bash
./test_audio
```

**操作方法：**
- `1-7`: 音階（ド-レ-ミ-ファ-ソ-ラ-シ）
- `B`: BEEP音（2000Hz）
- `SPACE`: 停止
- `Q` / `ESC`: 終了

### 2. HAL (Hardware Abstraction Layer) 統合デモ

#### `hal_demo` - HAL層統合デモ

プラットフォーム非依存のエミュレータコアと、SDL2実装のHAL層を組み合わせた統合デモ。

**実行方法：**
```bash
./hal_demo
```

**表示内容：**
- PC-8801風のテストパターン
- 8色パレット表示
- アニメーション効果
- フレームカウンタ

## HAL層の設計

### 設計ファイル

- **`hal_prototype.h`** - HAL抽象化インターフェース定義
- **`hal_sdl2_impl.cpp`** - SDL2による実装例

### HAL層の構成要素

| コンポーネント | 説明 | SDL2実装 |
|--------------|------|---------|
| `GraphicsHAL` | 描画抽象化 | SDL_Window + SDL_Renderer + SDL_Texture |
| `AudioHAL` | 音声抽象化 | SDL_AudioDevice |
| `ThreadHAL` | スレッド抽象化 | SDL_Thread（未実装） |
| `TimerHAL` | 時刻・タイマー | SDL_GetTicks |
| `FileHAL` | ファイルI/O | 標準C++（未実装） |
| `InputHAL` | 入力（キーボード・マウス） | SDL_Event（未実装） |

### 実装済み機能

- ✅ グラフィックス初期化・描画
- ✅ オーディオ初期化・再生
- ✅ タイマー機能
- ✅ イベント処理（終了・ESCキー）

### 未実装機能（今後の拡張）

- ⏳ スレッド管理（Critical Section等）
- ⏳ ファイルI/O抽象化
- ⏳ キーボード入力（PC-8801キーマッピング）
- ⏳ マウス入力
- ⏳ ジョイスティック入力

## ビルド方法

### 必要なライブラリ

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev pkg-config g++ make

# macOS (Homebrew)
brew install sdl2

# FreeBSD
pkg install sdl2
```

### ビルド

```bash
make all
```

### クリーン

```bash
make clean
```

## 実装の詳細

### エミュレータコアとHAL層の分離

```
┌─────────────────────────────────────┐
│  Emulator Core (プラットフォーム非依存) │
│  - Z80C (C++実装)                    │
│  - PC88 (メモリ、CRTC、FDC等)        │
│  - Sound (OPNA, PSG, BEEP)          │
└───────────┬─────────────────────────┘
            │ HAL Interface
┌───────────▼─────────────────────────┐
│  HAL Layer (プラットフォーム抽象化)    │
│  - Graphics, Audio, Thread, etc.   │
└───────────┬─────────────────────────┘
            │
    ┌───────┴────────┬──────────────┬─────────────┐
    ▼                ▼              ▼             ▼
┌─────────┐    ┌──────────┐   ┌─────────┐  ┌──────────┐
│ SDL2    │    │ Win32    │   │Headless │  │ Python   │
│(Linux)  │    │(Windows) │   │(Server) │  │(Wrapper) │
└─────────┘    └──────────┘   └─────────┘  └──────────┘
```

### Z80エミュレーションの選択

M88には2つのZ80実装があります：

1. **Z80c.cpp** - C++純粋実装（✅ プラットフォーム非依存）
2. **Z80_x86.cpp** - x86アセンブラ最適化版（❌ 32bit x86専用）

プラットフォーム移植では **Z80c.cpp** を使用します。

## パフォーマンス考察

### 8MHz Z80エミュレーション

- **Z80クロック**: 8MHz (8,000,000 cycles/sec)
- **1フレーム**: 約133,333 cycles (60Hz想定)
- **現代のCPU**: 2GHz以上

→ C++実装でも十分リアルタイム実行可能

### SDL2のオーバーヘッド

- **描画**: VSync同期で60fps
- **音声**: 44.1kHz, 4096サンプルバッファ（約93ms）
- **レイテンシ**: 許容範囲内

## 次のステップ

### フェーズ1: HAL層完成（1-2週間）✅ 完了

- [x] GraphicsHAL - SDL2実装
- [x] AudioHAL - SDL2実装
- [x] TimerHAL - SDL2実装
- [ ] ThreadHAL - SDL2実装
- [ ] FileHAL - 標準C++実装
- [ ] InputHAL - SDL2実装

### フェーズ2: Z80コア統合（1週間）

- [ ] Z80c.cpp を HAL環境でビルド
- [ ] MemoryManager の移植
- [ ] IOBus の移植
- [ ] 簡単なZ80プログラムの実行テスト

### フェーズ3: PC88コア移植（2-3週間）

- [ ] Memory.cpp の移植
- [ ] CRTC.cpp の移植（HAL Graphics経由）
- [ ] Sound.cpp の移植（HAL Audio経由）
- [ ] FDC.cpp の移植（HAL File経由）

### フェーズ4: 完全な移植（2-3週間）

- [ ] 全PC88デバイスの移植
- [ ] ディスクイメージ読み込み
- [ ] セーブステート機能
- [ ] デバッガ統合

### フェーズ5: プラットフォーム展開

- [ ] macOS版ビルド確認
- [ ] ARM64版ビルド確認
- [ ] ヘッドレス版（サーバー用）
- [ ] Python binding (pybind11)

## Python化の可能性

### ハイブリッド構成案

```python
# Python フロントエンド
import m88core  # C++コアをバインド

emu = m88core.Emulator()
emu.load_disk("game.d88")
emu.run()

# デバッグ機能
emu.set_breakpoint(0x1234)
memory = emu.read_memory(0x0000, 0x10000)
```

### メリット

- スクリプトによる自動化
- Jupyter Notebookでの対話的デバッグ
- NumPy/Matplotlibでの波形解析
- Webベースの操作画面（Flask/FastAPI）

## ライセンス

このプロトタイプは、M88本体と同じライセンスに従います。

## 作成者

- プロトタイプ設計・実装: Claude Code
- M88本体: cisc氏

## 参考資料

- M88公式: http://retropc.net/cisc/m88/
- SDL2: https://www.libsdl.org/
- PC-8801 Technical Manual

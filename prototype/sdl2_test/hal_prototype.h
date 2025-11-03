// HAL (Hardware Abstraction Layer) Prototype for M88
// プラットフォーム非依存化のための抽象レイヤー設計案
//
// 目的: Windows/Linux/macOS など、異なるプラットフォームで
//       M88エミュレータを動作させるための抽象化層

#ifndef HAL_PROTOTYPE_H
#define HAL_PROTOTYPE_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// Graphics HAL - 描画抽象化レイヤー
// ============================================================================

class GraphicsHAL {
public:
    virtual ~GraphicsHAL() {}

    // 初期化・終了
    virtual bool Initialize(int width, int height, const char* title) = 0;
    virtual void Shutdown() = 0;

    // フレームバッファ操作
    // PC-8801 の画面は 640x400, 8色パレット
    // buffer: RGB888 形式のピクセルデータ (width * height * 4 bytes)
    virtual bool LockFramebuffer(uint32_t** pixels, int* pitch) = 0;
    virtual void UnlockFramebuffer() = 0;

    // 描画更新
    virtual void Present() = 0;

    // イベント処理
    virtual bool PollEvents() = 0;  // false = 終了要求

    // パレット操作（PC-8801は8色パレット）
    virtual void SetPalette(int index, uint8_t r, uint8_t g, uint8_t b) = 0;
};

// ============================================================================
// Audio HAL - 音声抽象化レイヤー
// ============================================================================

class AudioHAL {
public:
    virtual ~AudioHAL() {}

    // 初期化・終了
    // sampleRate: サンプリングレート (例: 44100, 48000)
    // channels: チャンネル数 (1=モノラル, 2=ステレオ)
    // bufferSize: バッファサイズ（サンプル数）
    virtual bool Initialize(int sampleRate, int channels, int bufferSize) = 0;
    virtual void Shutdown() = 0;

    // 音声データ書き込み
    // samples: int16_t 形式の PCM データ
    // count: サンプル数（フレーム数 × チャンネル数）
    virtual bool WriteBuffer(const int16_t* samples, int count) = 0;

    // 再生制御
    virtual void Play() = 0;
    virtual void Pause() = 0;

    // バッファ状態取得
    virtual int GetBufferFreeSpace() = 0;  // 書き込み可能なサンプル数
};

// ============================================================================
// Thread HAL - スレッド抽象化レイヤー
// ============================================================================

typedef void* ThreadHandle;
typedef void* MutexHandle;
typedef void (*ThreadFunc)(void* arg);

class ThreadHAL {
public:
    virtual ~ThreadHAL() {}

    // スレッド操作
    virtual ThreadHandle CreateThread(ThreadFunc func, void* arg) = 0;
    virtual void JoinThread(ThreadHandle handle) = 0;
    virtual void DetachThread(ThreadHandle handle) = 0;
    virtual void Sleep(int milliseconds) = 0;

    // Mutex操作
    virtual MutexHandle CreateMutex() = 0;
    virtual void DestroyMutex(MutexHandle handle) = 0;
    virtual void Lock(MutexHandle handle) = 0;
    virtual void Unlock(MutexHandle handle) = 0;
};

// ============================================================================
// Timer HAL - 時刻・タイマー抽象化レイヤー
// ============================================================================

class TimerHAL {
public:
    virtual ~TimerHAL() {}

    // 高精度時刻取得
    // 戻り値: マイクロ秒単位のタイムスタンプ
    virtual uint64_t GetMicroseconds() = 0;

    // ミリ秒単位の時刻取得
    virtual uint32_t GetMilliseconds() = 0;

    // 指定時間待機
    virtual void DelayMicroseconds(uint64_t us) = 0;
    virtual void DelayMilliseconds(uint32_t ms) = 0;
};

// ============================================================================
// File HAL - ファイルI/O抽象化レイヤー
// ============================================================================

enum FileMode {
    FILE_READ = 0x01,
    FILE_WRITE = 0x02,
    FILE_CREATE = 0x04,
    FILE_APPEND = 0x08
};

typedef void* FileHandle;

class FileHAL {
public:
    virtual ~FileHAL() {}

    // ファイル操作
    virtual FileHandle Open(const char* path, int mode) = 0;
    virtual void Close(FileHandle handle) = 0;

    // 読み書き
    virtual size_t Read(FileHandle handle, void* buffer, size_t size) = 0;
    virtual size_t Write(FileHandle handle, const void* buffer, size_t size) = 0;

    // シーク
    virtual bool Seek(FileHandle handle, int64_t offset, int origin) = 0;
    virtual int64_t Tell(FileHandle handle) = 0;
    virtual int64_t GetSize(FileHandle handle) = 0;

    // ファイル・ディレクトリ情報
    virtual bool Exists(const char* path) = 0;
    virtual bool IsDirectory(const char* path) = 0;
};

// ============================================================================
// Input HAL - 入力抽象化レイヤー（キーボード・マウス）
// ============================================================================

enum KeyCode {
    KEY_UNKNOWN = 0,
    KEY_ESCAPE,
    KEY_SPACE,
    KEY_RETURN,
    // ... PC-8801 のキーマッピング
};

enum KeyState {
    KEY_RELEASED = 0,
    KEY_PRESSED = 1
};

class InputHAL {
public:
    virtual ~InputHAL() {}

    // キーボード状態取得
    virtual KeyState GetKeyState(KeyCode key) = 0;

    // マウス状態取得
    virtual void GetMousePosition(int* x, int* y) = 0;
    virtual bool GetMouseButton(int button) = 0;
};

// ============================================================================
// Platform HAL - プラットフォーム全体の抽象化
// ============================================================================

class PlatformHAL {
public:
    virtual ~PlatformHAL() {}

    // 各サブシステムへのアクセス
    virtual GraphicsHAL* GetGraphics() = 0;
    virtual AudioHAL* GetAudio() = 0;
    virtual ThreadHAL* GetThread() = 0;
    virtual TimerHAL* GetTimer() = 0;
    virtual FileHAL* GetFile() = 0;
    virtual InputHAL* GetInput() = 0;

    // プラットフォーム初期化・終了
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // イベント処理
    virtual bool ProcessEvents() = 0;  // false = 終了要求
};

// ============================================================================
// プラットフォーム実装の作成
// ============================================================================

// SDL2実装（Linux/Windows/macOS対応）
PlatformHAL* CreateSDL2Platform();

// Windows ネイティブ実装（既存コード）
PlatformHAL* CreateWin32Platform();

// ヘッドレス実装（サーバー・テスト用）
PlatformHAL* CreateHeadlessPlatform();

#endif // HAL_PROTOTYPE_H

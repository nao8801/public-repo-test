// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1997, 2001.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  Sound Implementation for SDL2

#include "SoundSDL2.h"
#include "pc88/pc88.h"
#include "pc88/config.h"
#include <stdio.h>
#include <string.h>

using namespace PC8801;

// ---------------------------------------------------------------------------
//  構築/消滅
//
SoundSDL2::SoundSDL2()
    : audio_device(0)
    , current_rate(0)
    , current_buflen(0)
    , initialized(false)
{
}

SoundSDL2::~SoundSDL2()
{
    Cleanup();
}

// ---------------------------------------------------------------------------
//  SDL2 オーディオコールバック
//  このコールバックはSDL2のオーディオスレッドから呼ばれる
//
void SDLCALL SoundSDL2::AudioCallback(void* userdata, Uint8* stream, int len)
{
    SoundDumpPipe* dumper = static_cast<SoundDumpPipe*>(userdata);
    if (!dumper) {
        // 音源がない場合は無音
        memset(stream, 0, len);
        return;
    }

    // バッファをSample*（int16_t*）にキャスト
    Sample* buffer = reinterpret_cast<Sample*>(stream);

    // サンプル数を計算（ステレオなので / 2 / sizeof(Sample)）
    int samples = len / (2 * sizeof(Sample));

    // dumperからデータを取得（dumperが内部で元の音源から取得し、WAVファイルに書き込む）
    int got = dumper->Get(buffer, samples);

    // 取得できなかった分は無音で埋める
    if (got < samples) {
        memset(buffer + got * 2, 0, (samples - got) * 2 * sizeof(Sample));
    }
}

// ---------------------------------------------------------------------------
//  初期化
//
bool SoundSDL2::Init(PC88* pc, uint rate, uint buflen)
{
    printf("Initializing SoundSDL2...\n");

    current_rate = rate;
    current_buflen = buflen;

    // デフォルト値設定
    if (rate == 0) {
        rate = 44100;  // 44.1kHz
    }
    if (buflen == 0) {
        buflen = 100;  // 100ms
    }

    printf("  - Sample rate: %d Hz\n", rate);
    printf("  - Buffer length: %d ms\n", buflen);

    // 基底クラス（Sound）の初期化
    // バッファサイズはサンプル単位で指定（rate * buflen / 1000）
    int bufsize = (rate * buflen / 1000) & ~15;  // 16の倍数に揃える
    printf("  - Buffer size: %d samples\n", bufsize);

    if (!Sound::Init(pc, rate, bufsize)) {
        fprintf(stderr, "Sound::Init failed\n");
        return false;
    }

    // SoundDumpPipeに元の音源を設定
    dumper.SetSource(GetSoundSource());

    // SDL2オーディオの初期化
    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = rate;                    // サンプリングレート
    want.format = AUDIO_S16SYS;          // 16bit signed (システムのエンディアン)
    want.channels = 2;                   // ステレオ
    want.samples = bufsize / 4;          // バッファサイズ（サンプル単位）
    want.callback = AudioCallback;       // コールバック関数
    want.userdata = &dumper;              // SoundDumpPipeを渡す（dumperが内部で元の音源から取得）

    printf("  - Opening SDL2 audio device...\n");
    audio_device = SDL_OpenAudioDevice(
        NULL,  // デフォルトデバイス
        0,     // 再生用（0=playback, 1=capture）
        &want,
        &have,
        0      // 変更を許可しない
    );

    if (audio_device == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        Sound::Cleanup();
        return false;
    }

    printf("  - Audio device opened successfully\n");
    printf("    - Format: %d Hz, %d channels, %d samples buffer\n",
           have.freq, have.channels, have.samples);

    // オーディオ再生開始
    SDL_PauseAudioDevice(audio_device, 0);
    printf("  - Audio playback started\n");

    initialized = true;
    return true;
}

// ---------------------------------------------------------------------------
//  後処理
//
void SoundSDL2::Cleanup()
{
    if (initialized) {
        printf("Cleaning up SoundSDL2...\n");

        if (audio_device) {
            printf("  - Stopping audio playback...\n");
            SDL_PauseAudioDevice(audio_device, 1);
            SDL_CloseAudioDevice(audio_device);
            audio_device = 0;
        }

        initialized = false;
    }

    // 基底クラスのクリーンアップ
    Sound::Cleanup();
}

// ---------------------------------------------------------------------------
//  設定更新
//
void SoundSDL2::ApplyConfig(const Config* config)
{
    // 基底クラスの設定更新を呼ぶ
    Sound::ApplyConfig(config);

    // SDL2版では動的なレート変更は複雑なので、
    // とりあえず基底クラスの設定更新のみ行う
    // 必要に応じて後で実装
}

// ---------------------------------------------------------------------------
//  WAV録音開始
//
bool SoundSDL2::DumpBegin(const char* filename)
{
    return dumper.DumpStart(filename);
}

// ---------------------------------------------------------------------------
//  WAV録音終了
//
bool SoundSDL2::DumpEnd()
{
    return dumper.DumpStop();
}

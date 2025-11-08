// SDL2 Audio Test for M88 Prototype (Debug Version)
// PC-8801 BEEP音とサイン波テスト - デバッグ情報追加版

#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

const int SAMPLE_RATE = 44100;
const int CHANNELS = 2;  // ステレオ
const int BUFFER_SIZE = 4096;

// オーディオコールバック用の状態
struct AudioContext {
    double phase;        // 位相（サイン波生成用）
    double frequency;    // 周波数
    double amplitude;    // 音量
    bool playing;        // 再生中フラグ
    int callback_calls;  // コールバック呼び出し回数
};

// オーディオコールバック関数
void AudioCallback(void* userdata, Uint8* stream, int len) {
    AudioContext* ctx = (AudioContext*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / (sizeof(int16_t) * CHANNELS);

    ctx->callback_calls++;

    // デバッグ: 最初の10回だけログ出力
    if (ctx->callback_calls <= 10) {
        printf("[DEBUG] AudioCallback #%d: len=%d, samples=%d, playing=%d\n",
               ctx->callback_calls, len, samples, ctx->playing);
    }

    if (!ctx->playing) {
        // 無音
        memset(stream, 0, len);
        return;
    }

    for (int i = 0; i < samples; i++) {
        // サイン波生成
        double sample = sin(ctx->phase) * ctx->amplitude;
        int16_t value = (int16_t)(sample * 32767.0);

        // ステレオ両チャンネルに同じ値を設定
        buffer[i * 2 + 0] = value;  // L
        buffer[i * 2 + 1] = value;  // R

        // 位相を進める
        ctx->phase += 2.0 * M_PI * ctx->frequency / SAMPLE_RATE;
        if (ctx->phase >= 2.0 * M_PI) {
            ctx->phase -= 2.0 * M_PI;
        }
    }
}

int main(int argc, char* argv[]) {
    printf("==============================================\n");
    printf("SDL2 Audio Test (Debug Version)\n");
    printf("==============================================\n\n");

    // SDL音声サブシステム初期化
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "❌ SDL音声初期化失敗: %s\n", SDL_GetError());
        return 1;
    }
    printf("✅ SDL音声初期化成功\n");

    // オーディオドライバ情報を表示
    const char* driver = SDL_GetCurrentAudioDriver();
    if (driver) {
        printf("🔊 使用中のオーディオドライバ: %s\n", driver);
    }

    // 利用可能なオーディオデバイスを列挙
    int audio_device_count = SDL_GetNumAudioDevices(0);
    printf("📡 利用可能なオーディオデバイス数: %d\n", audio_device_count);
    for (int i = 0; i < audio_device_count; i++) {
        const char* device_name = SDL_GetAudioDeviceName(i, 0);
        printf("   [%d] %s\n", i, device_name ? device_name : "(null)");
    }
    printf("\n");

    printf("サンプルレート: %d Hz\n", SAMPLE_RATE);
    printf("チャンネル数: %d (ステレオ)\n", CHANNELS);
    printf("バッファサイズ: %d samples\n", BUFFER_SIZE);
    printf("\n");
    printf("キー操作:\n");
    printf("  1-7: 音階（ド-シ）\n");
    printf("  B: BEEP音 (2000Hz)\n");
    printf("  T: テスト音（440Hz、自動再生開始）\n");
    printf("  SPACE: 停止\n");
    printf("  Q/ESC: 終了\n\n");

    // オーディオコンテキスト初期化
    AudioContext ctx;
    ctx.phase = 0.0;
    ctx.frequency = 440.0;  // A4 (ラ)
    ctx.amplitude = 0.3;    // 音量（0.0-1.0）
    ctx.playing = false;
    ctx.callback_calls = 0;

    // オーディオ仕様設定
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;  // 16bit signed
    want.channels = CHANNELS;
    want.samples = BUFFER_SIZE;
    want.callback = AudioCallback;
    want.userdata = &ctx;

    printf("要求するオーディオ仕様:\n");
    printf("  Frequency: %d Hz\n", want.freq);
    printf("  Format: AUDIO_S16SYS (16bit signed)\n");
    printf("  Channels: %d\n", want.channels);
    printf("  Samples: %d\n\n", want.samples);

    // オーディオデバイスオープン
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(
        NULL,  // デフォルトデバイス
        0,     // 再生用
        &want,
        &have,
        0      // 変更を許可しない
    );

    if (device == 0) {
        fprintf(stderr, "❌ オーディオデバイスオープン失敗: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("✅ オーディオデバイスオープン成功 (ID: %d)\n", device);
    printf("\n実際のオーディオ仕様:\n");
    printf("  Frequency: %d Hz", have.freq);
    if (have.freq != want.freq) printf(" ⚠️ 要求と異なる!");
    printf("\n");

    printf("  Format: ");
    if (have.format == AUDIO_S16SYS) {
        printf("AUDIO_S16SYS (16bit signed)");
    } else if (have.format == AUDIO_F32SYS) {
        printf("AUDIO_F32SYS (32bit float)");
    } else {
        printf("0x%04X", have.format);
    }
    if (have.format != want.format) printf(" ⚠️ 要求と異なる!");
    printf("\n");

    printf("  Channels: %d", have.channels);
    if (have.channels != want.channels) printf(" ⚠️ 要求と異なる!");
    printf("\n");

    printf("  Samples: %d", have.samples);
    if (have.samples != want.samples) printf(" ⚠️ 要求と異なる!");
    printf("\n\n");

    // オーディオ開始
    SDL_PauseAudioDevice(device, 0);
    printf("🎵 オーディオ再生開始（コールバック有効）\n");
    printf("準備完了。キーを押して音を出してください。\n");
    printf("※ 最初に 'T' キーを押すとテスト音が自動再生されます。\n\n");

    // イベントループ
    bool running = true;
    SDL_Event event;

    // 音階の周波数テーブル（C4-B4）
    const double notes[] = {
        261.63,  // C4 (ド)
        293.66,  // D4 (レ)
        329.63,  // E4 (ミ)
        349.23,  // F4 (ファ)
        392.00,  // G4 (ソ)
        440.00,  // A4 (ラ)
        493.88,  // B4 (シ)
    };
    const char* note_names[] = {
        "ド (C4)", "レ (D4)", "ミ (E4)", "ファ (F4)",
        "ソ (G4)", "ラ (A4)", "シ (B4)"
    };

    uint32_t last_status_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                printf("[KEY] Pressed: %s (0x%x)\n", SDL_GetKeyName(key), key);

                if (key == SDLK_q || key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_SPACE) {
                    ctx.playing = false;
                    printf("⏸️  停止 (playing = false)\n");
                } else if (key >= SDLK_1 && key <= SDLK_7) {
                    int index = key - SDLK_1;
                    ctx.frequency = notes[index];
                    ctx.phase = 0.0;
                    ctx.playing = true;
                    printf("🎵 %s (%.2f Hz) 再生中 (playing = true)\n",
                           note_names[index], ctx.frequency);
                } else if (key == SDLK_b) {
                    // PC-8801 BEEP音の周波数 (約2kHz)
                    ctx.frequency = 2000.0;
                    ctx.phase = 0.0;
                    ctx.playing = true;
                    printf("🎵 BEEP音 (2000 Hz) 再生中 (playing = true)\n");
                } else if (key == SDLK_t) {
                    // テスト音（440Hz自動再生）
                    ctx.frequency = 440.0;
                    ctx.phase = 0.0;
                    ctx.playing = true;
                    printf("🎵 テスト音 (440 Hz) 再生中 (playing = true)\n");
                    printf("    ※ 音が聞こえない場合、システムの音量設定を確認してください\n");
                }
            }
        }

        // 定期的にステータス表示
        uint32_t now = SDL_GetTicks();
        if (now - last_status_time >= 5000) {  // 5秒ごと
            printf("[STATUS] playing=%d, frequency=%.2f Hz, callbacks=%d\n",
                   ctx.playing, ctx.frequency, ctx.callback_calls);
            last_status_time = now;
        }

        SDL_Delay(10);
    }

    // クリーンアップ
    printf("\n終了処理中...\n");
    printf("総コールバック呼び出し回数: %d\n", ctx.callback_calls);
    SDL_CloseAudioDevice(device);
    SDL_Quit();

    printf("✅ 正常終了\n");
    return 0;
}

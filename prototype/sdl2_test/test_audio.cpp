// SDL2 Audio Test for M88 Prototype
// PC-8801 BEEP音とサイン波テスト

#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

const int SAMPLE_RATE = 44100;
const int CHANNELS = 2;  // ステレオ
const int BUFFER_SIZE = 4096;

// オーディオコールバック用の状態
struct AudioContext {
    double phase;        // 位相（サイン波生成用）
    double frequency;    // 周波数
    double amplitude;    // 音量
    bool playing;        // 再生中フラグ
};

// オーディオコールバック関数
void AudioCallback(void* userdata, Uint8* stream, int len) {
    AudioContext* ctx = (AudioContext*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / (sizeof(int16_t) * CHANNELS);

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
    // SDL音声サブシステム初期化
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL音声初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL2 オーディオテスト起動\n");
    printf("サンプルレート: %d Hz\n", SAMPLE_RATE);
    printf("チャンネル数: %d (ステレオ)\n", CHANNELS);
    printf("\n");
    printf("キー操作:\n");
    printf("  1-7: 音階（ド-シ）\n");
    printf("  SPACE: 停止\n");
    printf("  Q: 終了\n\n");

    // オーディオコンテキスト初期化
    AudioContext ctx;
    ctx.phase = 0.0;
    ctx.frequency = 440.0;  // A4 (ラ)
    ctx.amplitude = 0.3;    // 音量（0.0-1.0）
    ctx.playing = false;

    // オーディオ仕様設定
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;  // 16bit signed
    want.channels = CHANNELS;
    want.samples = BUFFER_SIZE;
    want.callback = AudioCallback;
    want.userdata = &ctx;

    // オーディオデバイスオープン
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(
        NULL,  // デフォルトデバイス
        0,     // 再生用
        &want,
        &have,
        0      // 変更を許可しない
    );

    if (device == 0) {
        fprintf(stderr, "オーディオデバイスオープン失敗: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("オーディオデバイス設定:\n");
    printf("  サンプルレート: %d Hz\n", have.freq);
    printf("  チャンネル数: %d\n", have.channels);
    printf("  バッファサイズ: %d samples\n", have.samples);
    printf("\n準備完了。キーを押して音を出してください。\n");

    // オーディオ開始
    SDL_PauseAudioDevice(device, 0);

    // イベントループ（標準入力から）
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

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_q || key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_SPACE) {
                    ctx.playing = false;
                    printf("停止\n");
                } else if (key >= SDLK_1 && key <= SDLK_7) {
                    int index = key - SDLK_1;
                    ctx.frequency = notes[index];
                    ctx.phase = 0.0;
                    ctx.playing = true;
                    printf("♪ %s (%.2f Hz) 再生中\n", note_names[index], ctx.frequency);
                } else if (key == SDLK_b) {
                    // PC-8801 BEEP音の周波数 (約2kHz)
                    ctx.frequency = 2000.0;
                    ctx.phase = 0.0;
                    ctx.playing = true;
                    printf("♪ BEEP音 (2000 Hz) 再生中\n");
                }
            }
        }

        SDL_Delay(10);
    }

    // クリーンアップ
    SDL_CloseAudioDevice(device);
    SDL_Quit();

    printf("終了しました\n");
    return 0;
}

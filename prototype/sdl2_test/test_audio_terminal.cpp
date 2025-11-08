// SDL2 Audio Test for M88 Prototype (Terminal Version)
// PC-8801 BEEP音とサイン波テスト - 標準入力版

#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

const int SAMPLE_RATE = 44100;
const int CHANNELS = 2;  // ステレオ
const int BUFFER_SIZE = 4096;

// オーディオコールバック用の状態
struct AudioContext {
    double phase;        // 位相（サイン波生成用）
    double frequency;    // 周波数
    double amplitude;    // 音量
    volatile bool playing;        // 再生中フラグ
    volatile int callback_calls;  // コールバック呼び出し回数
};

// オーディオコールバック関数
void AudioCallback(void* userdata, Uint8* stream, int len) {
    AudioContext* ctx = (AudioContext*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / (sizeof(int16_t) * CHANNELS);

    ctx->callback_calls++;

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

// ターミナルを非カノニカルモードに設定（1文字ずつ読み取る）
void SetNonCanonicalMode(bool enable, struct termios* oldt) {
    static struct termios newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, oldt);
        newt = *oldt;
        newt.c_lflag &= ~(ICANON | ECHO);  // カノニカルモードとエコーを無効化
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // 非ブロッキングモードに設定
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    } else {
        // 元に戻す
        tcsetattr(STDIN_FILENO, TCSANOW, oldt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
}

int main(int argc, char* argv[]) {
    printf("==============================================\n");
    printf("SDL2 Audio Test (Terminal Version)\n");
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
    printf("   サンプルレート: %d Hz\n", have.freq);
    printf("   チャンネル数: %d\n", have.channels);
    printf("   バッファサイズ: %d samples\n\n", have.samples);

    // オーディオ開始
    SDL_PauseAudioDevice(device, 0);
    printf("🎵 オーディオ再生開始（コールバック有効）\n\n");

    printf("==============================================\n");
    printf("キー操作（ターミナルで直接入力）:\n");
    printf("==============================================\n");
    printf("  1-7: 音階（ド-シ）\n");
    printf("  b/B: BEEP音 (2000Hz)\n");
    printf("  t/T: テスト音 (440Hz) ← まずこれを試してください！\n");
    printf("  SPACE: 停止\n");
    printf("  q/Q: 終了\n");
    printf("==============================================\n\n");
    printf("準備完了。キーを押してください（Enterは不要）\n");
    printf("※ 最初に 't' または 'T' を押すとテスト音が鳴ります\n\n");

    // ターミナルを非カノニカルモードに設定
    struct termios oldt;
    SetNonCanonicalMode(true, &oldt);

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

    bool running = true;
    uint32_t last_status_time = SDL_GetTicks();
    int last_callback_count = 0;

    while (running) {
        // 標準入力から1文字読み取り（非ブロッキング）
        char ch = 0;
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        if (n > 0) {
            printf("\n[入力] '%c' (0x%02x)\n", ch, (unsigned char)ch);

            if (ch == 'q' || ch == 'Q') {
                printf("終了します...\n");
                running = false;
            } else if (ch == ' ') {
                ctx.playing = false;
                printf("⏸️  停止 (playing = false)\n");
            } else if (ch >= '1' && ch <= '7') {
                int index = ch - '1';
                ctx.frequency = notes[index];
                ctx.phase = 0.0;
                ctx.playing = true;
                printf("🎵 %s (%.2f Hz) 再生中 (playing = true)\n",
                       note_names[index], ctx.frequency);
                printf("   ※ 音が聞こえない場合、システム音量を確認してください\n");
            } else if (ch == 'b' || ch == 'B') {
                ctx.frequency = 2000.0;
                ctx.phase = 0.0;
                ctx.playing = true;
                printf("🎵 BEEP音 (2000 Hz) 再生中 (playing = true)\n");
            } else if (ch == 't' || ch == 'T') {
                ctx.frequency = 440.0;
                ctx.phase = 0.0;
                ctx.playing = true;
                printf("🎵 テスト音 (440 Hz) 再生中 (playing = true)\n");
                printf("   ※ 音が聞こえますか？\n");
            } else {
                printf("   （未対応のキー）\n");
            }
        }

        // 定期的にステータス表示
        uint32_t now = SDL_GetTicks();
        if (now - last_status_time >= 5000) {  // 5秒ごと
            int callbacks_per_sec = (ctx.callback_calls - last_callback_count) / 5;
            printf("\n[STATUS] playing=%d, freq=%.2f Hz, callbacks=%d (%d/sec)\n",
                   ctx.playing, ctx.frequency, ctx.callback_calls, callbacks_per_sec);
            last_status_time = now;
            last_callback_count = ctx.callback_calls;
        }

        SDL_Delay(10);
    }

    // ターミナル設定を元に戻す
    SetNonCanonicalMode(false, &oldt);

    // クリーンアップ
    printf("\n終了処理中...\n");
    printf("総コールバック呼び出し回数: %d\n", ctx.callback_calls);
    SDL_CloseAudioDevice(device);
    SDL_Quit();

    printf("✅ 正常終了\n");
    return 0;
}

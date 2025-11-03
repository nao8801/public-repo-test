// HAL SDL2 Implementation - SDL2を使ったHAL層の実装例
//
// このファイルは、HAL抽象化レイヤーのSDL2実装を示すプロトタイプです。
// 実際のM88移植では、これを基に実装を進めます。

#include "hal_prototype.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// SDL2 Graphics Implementation
// ============================================================================

class SDL2Graphics : public GraphicsHAL {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int width;
    int height;
    uint32_t palette[8];  // PC-8801の8色パレット

public:
    SDL2Graphics() : window(nullptr), renderer(nullptr), texture(nullptr),
                     width(0), height(0) {
        // デフォルトパレット（PC-8801風）
        palette[0] = 0x000000;  // 黒
        palette[1] = 0x0000FF;  // 青
        palette[2] = 0xFF0000;  // 赤
        palette[3] = 0xFF00FF;  // マゼンタ
        palette[4] = 0x00FF00;  // 緑
        palette[5] = 0x00FFFF;  // シアン
        palette[6] = 0xFFFF00;  // 黄
        palette[7] = 0xFFFFFF;  // 白
    }

    virtual ~SDL2Graphics() {
        Shutdown();
    }

    virtual bool Initialize(int w, int h, const char* title) override {
        width = w;
        height = h;

        // ウィンドウ作成
        window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN
        );

        if (!window) {
            fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        // レンダラー作成
        renderer = SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

        if (!renderer) {
            fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            window = nullptr;
            return false;
        }

        // テクスチャ作成
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING,
            width, height
        );

        if (!texture) {
            fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            renderer = nullptr;
            window = nullptr;
            return false;
        }

        printf("SDL2 Graphics initialized: %dx%d\n", width, height);
        return true;
    }

    virtual void Shutdown() override {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }

    virtual bool LockFramebuffer(uint32_t** pixels, int* pitch) override {
        if (!texture) return false;

        void* raw_pixels;
        int result = SDL_LockTexture(texture, nullptr, &raw_pixels, pitch);
        if (result < 0) {
            fprintf(stderr, "SDL_LockTexture failed: %s\n", SDL_GetError());
            return false;
        }

        *pixels = (uint32_t*)raw_pixels;
        return true;
    }

    virtual void UnlockFramebuffer() override {
        if (texture) {
            SDL_UnlockTexture(texture);
        }
    }

    virtual void Present() override {
        if (!renderer || !texture) return;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    virtual bool PollEvents() override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
            }
        }
        return true;
    }

    virtual void SetPalette(int index, uint8_t r, uint8_t g, uint8_t b) override {
        if (index >= 0 && index < 8) {
            palette[index] = (r << 16) | (g << 8) | b;
        }
    }

    uint32_t GetPaletteColor(int index) {
        if (index >= 0 && index < 8) {
            return palette[index];
        }
        return 0;
    }
};

// ============================================================================
// SDL2 Audio Implementation
// ============================================================================

class SDL2Audio : public AudioHAL {
private:
    SDL_AudioDeviceID device;
    int sampleRate;
    int channels;
    int bufferSize;
    bool initialized;

    static void AudioCallback(void* userdata, uint8_t* stream, int len) {
        // 簡易実装：無音を出力
        // 実際のM88では、ここでFM音源・PSG・BEEPなどのミキシング結果を出力
        memset(stream, 0, len);
    }

public:
    SDL2Audio() : device(0), sampleRate(0), channels(0),
                  bufferSize(0), initialized(false) {}

    virtual ~SDL2Audio() {
        Shutdown();
    }

    virtual bool Initialize(int rate, int ch, int bufSize) override {
        sampleRate = rate;
        channels = ch;
        bufferSize = bufSize;

        SDL_AudioSpec want, have;
        SDL_zero(want);
        want.freq = sampleRate;
        want.format = AUDIO_S16SYS;
        want.channels = channels;
        want.samples = bufferSize;
        want.callback = AudioCallback;
        want.userdata = this;

        device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);

        if (device == 0) {
            fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
            return false;
        }

        printf("SDL2 Audio initialized: %d Hz, %d ch, %d samples\n",
               have.freq, have.channels, have.samples);

        initialized = true;
        return true;
    }

    virtual void Shutdown() override {
        if (device) {
            SDL_CloseAudioDevice(device);
            device = 0;
        }
        initialized = false;
    }

    virtual bool WriteBuffer(const int16_t* samples, int count) override {
        // SDL2のオーディオキューに書き込む
        // 実際の実装では、コールバック方式とキュー方式を選択できる
        if (!initialized) return false;

        // プロトタイプなので簡易実装
        return true;
    }

    virtual void Play() override {
        if (device) {
            SDL_PauseAudioDevice(device, 0);
        }
    }

    virtual void Pause() override {
        if (device) {
            SDL_PauseAudioDevice(device, 1);
        }
    }

    virtual int GetBufferFreeSpace() override {
        // 簡易実装
        return bufferSize;
    }
};

// ============================================================================
// SDL2 Timer Implementation
// ============================================================================

class SDL2Timer : public TimerHAL {
public:
    virtual uint64_t GetMicroseconds() override {
        // SDL2は1/1000秒単位なので、マイクロ秒に変換
        return SDL_GetTicks() * 1000ULL;
    }

    virtual uint32_t GetMilliseconds() override {
        return SDL_GetTicks();
    }

    virtual void DelayMicroseconds(uint64_t us) override {
        SDL_Delay(us / 1000);
    }

    virtual void DelayMilliseconds(uint32_t ms) override {
        SDL_Delay(ms);
    }
};

// ============================================================================
// SDL2 Platform Implementation
// ============================================================================

class SDL2Platform : public PlatformHAL {
private:
    SDL2Graphics graphics;
    SDL2Audio audio;
    SDL2Timer timer;
    bool initialized;

public:
    SDL2Platform() : initialized(false) {}

    virtual ~SDL2Platform() {
        Shutdown();
    }

    virtual bool Initialize() override {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
            fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }

        initialized = true;
        printf("SDL2 Platform initialized\n");
        return true;
    }

    virtual void Shutdown() override {
        if (initialized) {
            graphics.Shutdown();
            audio.Shutdown();
            SDL_Quit();
            initialized = false;
            printf("SDL2 Platform shutdown\n");
        }
    }

    virtual GraphicsHAL* GetGraphics() override { return &graphics; }
    virtual AudioHAL* GetAudio() override { return &audio; }
    virtual ThreadHAL* GetThread() override { return nullptr; }  // 未実装
    virtual TimerHAL* GetTimer() override { return &timer; }
    virtual FileHAL* GetFile() override { return nullptr; }      // 未実装
    virtual InputHAL* GetInput() override { return nullptr; }    // 未実装

    virtual bool ProcessEvents() override {
        return graphics.PollEvents();
    }
};

// ============================================================================
// Factory Function
// ============================================================================

PlatformHAL* CreateSDL2Platform() {
    return new SDL2Platform();
}

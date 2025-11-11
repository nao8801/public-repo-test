// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1997, 2001.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  Sound Implementation for SDL2

#pragma once

#include <SDL2/SDL.h>
#include "pc88/sound.h"
#include "common/types.h"
#include "SoundDumpPipe.h"

class PC88;

namespace PC8801
{

class SoundSDL2 : public Sound
{
public:
    SoundSDL2();
    ~SoundSDL2();

    bool Init(PC88* pc, uint rate, uint buflen);
    void Cleanup();

    void ApplyConfig(const Config* config);

    // WAV録音機能
    bool DumpBegin(const char* filename);
    bool DumpEnd();
    bool IsDumping() { return dumper.IsDumping(); }

private:
    static void SDLCALL AudioCallback(void* userdata, Uint8* stream, int len);

    SDL_AudioDeviceID audio_device;
    uint current_rate;
    uint current_buflen;
    bool initialized;
    
    SoundDumpPipe dumper;  // WAV録音用パイプ
};

}

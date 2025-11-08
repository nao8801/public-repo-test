// ---------------------------------------------------------------------------
//  DrawSDL2 - SDL2-based drawing implementation for M88
//  Cross-platform graphics rendering using SDL2
// ---------------------------------------------------------------------------

#pragma once

#include "draw.h"
#include <SDL2/SDL.h>

class DrawSDL2 : public Draw
{
public:
    DrawSDL2();
    virtual ~DrawSDL2();

    // Draw interface implementation
    virtual bool Init(uint width, uint height, uint bpp) override;
    virtual bool Cleanup() override;

    virtual bool Lock(uint8** pimage, int* pbpl) override;
    virtual bool Unlock() override;

    virtual uint GetStatus() override;
    virtual void Resize(uint width, uint height) override;
    virtual void DrawScreen(const Region& region) override;
    virtual void SetPalette(uint index, uint nents, const Palette* pal) override;
    virtual void Flip() override;
    virtual bool SetFlipMode(bool flip) override;

    // SDL2-specific
    bool ProcessEvents();  // Returns false if quit requested
    SDL_Window* GetWindow() const { return window; }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    uint8* framebuffer;
    int width;
    int height;
    int bpp;
    int pitch;  // bytes per line

    uint status_flags;
    bool flip_mode;
    bool locked;

    // Palette (PC-8801 uses 8 colors + background)
    SDL_Color palette[256];
    bool palette_changed;
};

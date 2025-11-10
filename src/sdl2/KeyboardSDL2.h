// ---------------------------------------------------------------------------
//  M88 - PC88 emulator
//  Copyright (c) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  PC88 Keyboard Interface Emulation for SDL2
// ---------------------------------------------------------------------------

#pragma once

#include "device.h"
#include <SDL2/SDL.h>

// ---------------------------------------------------------------------------
namespace PC8801
{

class Config;

class KeyboardSDL2 : public Device
{
public:
	enum
	{
		reset = 0, vsync,
		in = 0,
	};

public:
	KeyboardSDL2();
	~KeyboardSDL2();
	bool Init();
	void ApplyConfig(const Config* config);

	uint IOCALL In(uint port);
	void IOCALL VSync(uint, uint data);
	void IOCALL Reset(uint=0, uint=0);

	void Activate(bool);
	void KeyDown(SDL_Keycode keycode);
	void KeyUp(SDL_Keycode keycode);

	const Descriptor* IFCALL GetDesc() const { return &descriptor; }

private:
	struct Key
	{
		SDL_Keycode sdl_key;  // SDL_Keycode (32-bit)
		uint8 pc88_row;       // PC-8801 key matrix row (0-15)
		uint8 pc88_col;       // PC-8801 key matrix column (0-7)
	};

	void UpdateKeyMatrix();

	static const Key KeyTable[];
	static const int KeyTableSize;

	bool active;
	int keyport[16];           // Key matrix cache (one byte per row)
	bool keystate[512];        // SDL key state (indexed by SDL_Scancode)

private:
	static const Descriptor descriptor;
	static const InFuncPtr  indef[];
	static const OutFuncPtr outdef[];
};

}

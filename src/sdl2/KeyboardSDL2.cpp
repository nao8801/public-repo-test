// ---------------------------------------------------------------------------
//  M88 - PC88 emulator
//  Copyright (c) cisc 1998, 1999.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  PC88 Keyboard Interface Emulation for SDL2
// ---------------------------------------------------------------------------

#include "headers.h"
#include "KeyboardSDL2.h"
#include "pc88/config.h"
#include <cstring>
#include <stdio.h>

using namespace PC8801;

// ---------------------------------------------------------------------------
//  Key mapping table: SDL_Keycode -> PC-8801 key matrix (row, col)
//  Based on WinKeyIF KeyTable101
//
//  PC-8801 Key Matrix Layout (16 rows x 8 columns):
//  Row 0: NUM0 NUM1 NUM2 NUM3 NUM4 NUM5 NUM6 NUM7
//  Row 1: NUM8 NUM9 NUM* NUM+ NUM= NUM, NUM. RET(numpad)
//  Row 2: @ A B C D E F G
//  Row 3: H I J K L M N O
//  Row 4: P Q R S T U V W
//  Row 5: X Y Z [ \ ] ^ -
//  Row 6: 0 1 2 3 4 5 6 7
//  Row 7: 8 9 : ; , . / _
//  Row 8: CLR ↑ → BS GRPH KANA SHIFT CTRL
//  Row 9: STOP F1 F2 F3 F4 F5 SPACE ESC
//  Row 10-15: Additional function keys and special keys
// ---------------------------------------------------------------------------

const KeyboardSDL2::Key KeyboardSDL2::KeyTable[] =
{
	// Numeric keys (row 6-7)
	{ SDLK_0, 6, 0 },
	{ SDLK_1, 6, 1 },
	{ SDLK_2, 6, 2 },
	{ SDLK_3, 6, 3 },
	{ SDLK_4, 6, 4 },
	{ SDLK_5, 6, 5 },
	{ SDLK_6, 6, 6 },
	{ SDLK_7, 6, 7 },
	{ SDLK_8, 7, 0 },
	{ SDLK_9, 7, 1 },

	// Alphabet keys (rows 2-5)
	{ SDLK_a, 2, 1 },
	{ SDLK_b, 2, 2 },
	{ SDLK_c, 2, 3 },
	{ SDLK_d, 2, 4 },
	{ SDLK_e, 2, 5 },
	{ SDLK_f, 2, 6 },
	{ SDLK_g, 2, 7 },
	{ SDLK_h, 3, 0 },
	{ SDLK_i, 3, 1 },
	{ SDLK_j, 3, 2 },
	{ SDLK_k, 3, 3 },
	{ SDLK_l, 3, 4 },
	{ SDLK_m, 3, 5 },
	{ SDLK_n, 3, 6 },
	{ SDLK_o, 3, 7 },
	{ SDLK_p, 4, 0 },
	{ SDLK_q, 4, 1 },
	{ SDLK_r, 4, 2 },
	{ SDLK_s, 4, 3 },
	{ SDLK_t, 4, 4 },
	{ SDLK_u, 4, 5 },
	{ SDLK_v, 4, 6 },
	{ SDLK_w, 4, 7 },
	{ SDLK_x, 5, 0 },
	{ SDLK_y, 5, 1 },
	{ SDLK_z, 5, 2 },

	// Symbol keys
	{ SDLK_MINUS,     5, 7 },  // - (minus)
	{ SDLK_EQUALS,    5, 6 },  // ^ (caret on US keyboard, mapped to PC-88 ^)
	{ SDLK_LEFTBRACKET,  5, 3 },  // [
	{ SDLK_RIGHTBRACKET, 5, 5 },  // ]
	{ SDLK_BACKSLASH,    5, 4 },  // backslash
	{ SDLK_SEMICOLON,    7, 3 },  // ;
	{ SDLK_QUOTE,        7, 2 },  // : (PC-88)
	{ SDLK_COMMA,        7, 4 },  // ,
	{ SDLK_PERIOD,       7, 5 },  // .
	{ SDLK_SLASH,        7, 6 },  // /
	{ SDLK_BACKQUOTE,    2, 0 },  // @ (on PC-88)

	// Control keys
	{ SDLK_SPACE,     9, 6 },  // SPACE
	{ SDLK_RETURN,    1, 7 },  // RETURN (main keyboard)
	{ SDLK_BACKSPACE, 8, 3 },  // BS (backspace)
	{ SDLK_ESCAPE,    9, 7 },  // ESC
	{ SDLK_TAB,      10, 1 },  // TAB

	// Arrow keys (row 8)
	{ SDLK_UP,    8, 1 },  // ↑
	{ SDLK_DOWN,  0, 2 },  // ↓ (mapped to NUM2 position for compatibility)
	{ SDLK_LEFT,  0, 4 },  // ← (mapped to NUM4 position)
	{ SDLK_RIGHT, 8, 2 },  // →

	// Function keys (row 9 and beyond)
	{ SDLK_F1,  9, 1 },
	{ SDLK_F2,  9, 2 },
	{ SDLK_F3,  9, 3 },
	{ SDLK_F4,  9, 4 },
	{ SDLK_F5,  9, 5 },
	{ SDLK_F6,  9, 1 },  // F6 also maps to F1 (SHIFT+F1)
	{ SDLK_F7,  9, 2 },  // F7 also maps to F2 (SHIFT+F2)
	{ SDLK_F8,  9, 3 },  // F8 also maps to F3 (SHIFT+F3)
	{ SDLK_F9,  9, 4 },  // F9 also maps to F4 (SHIFT+F4)
	{ SDLK_F10, 9, 5 },  // F10 also maps to F5 (SHIFT+F5)
	{ SDLK_F11, 9, 0 },  // STOP
	{ SDLK_F12, 10, 7 }, // COPY

	// Modifier keys
	{ SDLK_LSHIFT, 8, 6 },  // SHIFT (left)
	{ SDLK_RSHIFT, 8, 6 },  // SHIFT (right, same as left)
	{ SDLK_LCTRL,  8, 7 },  // CTRL (left)
	{ SDLK_RCTRL,  8, 7 },  // CTRL (right, same as left)
	{ SDLK_LALT,   8, 4 },  // GRPH (mapped to Alt)
	{ SDLK_RALT,   8, 4 },  // GRPH (mapped to Alt)

	// Numpad keys (row 0-1)
	{ SDLK_KP_0, 0, 0 },
	{ SDLK_KP_1, 0, 1 },
	{ SDLK_KP_2, 0, 2 },
	{ SDLK_KP_3, 0, 3 },
	{ SDLK_KP_4, 0, 4 },
	{ SDLK_KP_5, 0, 5 },
	{ SDLK_KP_6, 0, 6 },
	{ SDLK_KP_7, 0, 7 },
	{ SDLK_KP_8, 1, 0 },
	{ SDLK_KP_9, 1, 1 },
	{ SDLK_KP_MULTIPLY, 1, 2 },  // NUM*
	{ SDLK_KP_PLUS,     1, 3 },  // NUM+
	{ SDLK_KP_PERIOD,   1, 6 },  // NUM.
	{ SDLK_KP_ENTER,    1, 7 },  // RET (numpad)

	// Special keys
	{ SDLK_HOME,   8, 0 },  // CLR
	{ SDLK_DELETE, 10, 4 }, // DEL
	{ SDLK_INSERT, 10, 3 }, // INS

	// Terminator
	{ SDLK_UNKNOWN, 0, 0 }
};

const int KeyboardSDL2::KeyTableSize = sizeof(KeyTable) / sizeof(Key) - 1;

// ---------------------------------------------------------------------------
//  Descriptor
//
const Device::Descriptor KeyboardSDL2::descriptor =
{
	KeyboardSDL2::indef, KeyboardSDL2::outdef
};

const Device::InFuncPtr KeyboardSDL2::indef[] =
{
	STATIC_CAST(Device::InFuncPtr, &KeyboardSDL2::In),
	0
};

const Device::OutFuncPtr KeyboardSDL2::outdef[] =
{
	STATIC_CAST(Device::OutFuncPtr, &KeyboardSDL2::Reset),
	STATIC_CAST(Device::OutFuncPtr, &KeyboardSDL2::VSync),
	0
};

// ---------------------------------------------------------------------------
//  Constructor/Destructor
//
KeyboardSDL2::KeyboardSDL2()
: Device(0)
{
	for (int i = 0; i < 16; i++)
	{
		keyport[i] = 0xFF;  // All keys released (negative logic)
	}
	keystate.clear();
	active = true;
}

KeyboardSDL2::~KeyboardSDL2()
{
}

// ---------------------------------------------------------------------------
//  Initialize
//
bool KeyboardSDL2::Init()
{
	return true;
}

// ---------------------------------------------------------------------------
//  Reset (BASIC mode change)
//
void IOCALL KeyboardSDL2::Reset(uint, uint)
{
	// Clear all key states
	for (int i = 0; i < 16; i++)
	{
		keyport[i] = 0xFF;
	}
	keystate.clear();
}

// ---------------------------------------------------------------------------
//  Apply configuration
//
void KeyboardSDL2::ApplyConfig(const Config* config)
{
	// Future: handle different keyboard types (101/106/98)
}

// ---------------------------------------------------------------------------
//  VSync processing
//
void IOCALL KeyboardSDL2::VSync(uint, uint d)
{
	if (d && active)
	{
		// Clear key matrix cache (invalidate on VSync)
		for (int i = 0; i < 16; i++)
		{
			keyport[i] = -1;
		}
	}
}

// ---------------------------------------------------------------------------
//  Key input (read key matrix)
//
uint IOCALL KeyboardSDL2::In(uint port)
{
	port &= 0x0F;  // Only 16 rows (0x00-0x0F)

	if (active)
	{
		int r = keyport[port];
		if (r == -1)  // Cache miss
		{
			// Build the key matrix byte for this row
			r = 0xFF;  // All keys released (negative logic: 1=released, 0=pressed)

			// Scan all keys in the key table
			for (int i = 0; i < KeyTableSize; i++)
			{
				const Key& key = KeyTable[i];
				if (key.pc88_row == port)
				{
					// Check if this SDL keycode is pressed
					if (key.sdl_keycode != SDLK_UNKNOWN)
					{
						auto it = keystate.find(key.sdl_keycode);
						if (it != keystate.end() && it->second)
						{
							// Key is pressed, clear the corresponding bit (negative logic)
							r &= ~(1 << key.pc88_col);
						}
					}
				}
			}

			keyport[port] = r;  // Cache the result
		}
		return uint8(r);
	}
	else
	{
		return 0xFF;  // All keys released when inactive
	}
}

// ---------------------------------------------------------------------------
//  Activate/deactivate keyboard
//
void KeyboardSDL2::Activate(bool yes)
{
	active = yes;
	if (active)
	{
		keystate.clear();
		for (int i = 0; i < 16; i++)
		{
			keyport[i] = -1;
		}
	}
}

// ---------------------------------------------------------------------------
//  Key down event
//
void KeyboardSDL2::KeyDown(SDL_Keycode keycode)
{
	if (keycode != SDLK_UNKNOWN)
	{
		keystate[keycode] = true;
	}
}

// ---------------------------------------------------------------------------
//  Key up event
//
void KeyboardSDL2::KeyUp(SDL_Keycode keycode)
{
	if (keycode != SDLK_UNKNOWN)
	{
		keystate[keycode] = false;
	}
}

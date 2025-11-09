// ---------------------------------------------------------------------------
//	romeo/piccolo.h - SDL2 version (stub)
//	SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//	Romeo/Piccolo hardware interface - stub version
//
//	Note: Romeo/Piccolo is a real hardware interface for connecting
//	      to actual PC-8801 hardware. SDL2 version doesn't support this.

#pragma once

// Stub definitions for Piccolo hardware
#define PICCOLO_YMF288 0

class Piccolo
{
public:
    static bool Available() { return false; }
    static void DeleteInstance() {}
    static Piccolo* GetInstance() { return nullptr; }
    void Connect() {}
    bool IsDriverBased() { return false; }
    int GetCurrentTime() { return 0; }
    int GetChip(int type, PiccoloChip** chip) { return -1; }  // Always fail (no hardware)
};

class PiccoloChip
{
public:
    void Out(int addr, int data) {}
    void Reset(bool=false) {}
    void SetReg(unsigned int addr, unsigned int data, unsigned int=0) {}
};

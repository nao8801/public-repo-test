// Platform-independent type definitions
// This file provides common type definitions for cross-platform builds
//
// For Windows builds, use src/win32/types.h instead

#pragma once

#include <stdint.h>

// Endianness
#ifndef ENDIAN_IS_BIG
#define ENDIAN_IS_SMALL
#endif

// Fixed-size types
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

// 8 bit numbers packed together
typedef uint32 packed;
#define PACK(p) ((p) | ((p) << 8) | ((p) << 16) | ((p) << 24))

// Pointer-sized integer type
#ifdef _WIN32
    #include <windows.h>
    typedef LONG_PTR intpointer;
#else
    #include <stdint.h>
    typedef intptr_t intpointer;
#endif

// Function pointer ID bit
// (Required for x86 version of Z80 engine)
#if defined(_WIN64) || !defined(_WIN32)
    #undef PTR_IDBIT
#else
    #if defined(_DEBUG)
        #define PTR_IDBIT	0x80000000
    #else
        #define PTR_IDBIT	0x1
    #endif
#endif

// Allow unaligned memory access (x86/x64 specific)
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define ALLOWBOUNDARYACCESS
#endif

// Use x86 version of Z80 engine (Windows 32bit only)
#if defined(_WIN32) && !defined(_WIN64)
#define USE_Z80_X86
#endif

// Use new C++ casts
#define USE_NEW_CAST

// Calling conventions
#ifdef _WIN32
    // Windows: Use __stdcall
    #define MEMCALL __stdcall
    #ifndef IFCALL
        #define IFCALL __stdcall
    #endif
    #ifndef IOCALL
        #define IOCALL __stdcall
    #endif
    // Windows: 'interface' is a keyword
#else
    // Linux/Unix: No calling convention needed
    #define MEMCALL
    #ifndef IFCALL
        #define IFCALL
    #endif
    #ifndef IOCALL
        #define IOCALL
    #endif
    #ifndef __stdcall
        #define __stdcall
    #endif
    // Linux/Unix: 'interface' → 'struct'
    #ifndef interface
        #define interface struct
    #endif

    // Linux/Unix: Stub Windows types
    typedef const void* REFIID;
    typedef void* HWND;
    typedef unsigned int UINT;
    typedef uintptr_t WPARAM;
    typedef intptr_t LPARAM;
    typedef void PROPSHEETPAGE;
#endif

// Cast macros
#if defined(USE_NEW_CAST) && defined(__cplusplus)
    #define STATIC_CAST(t, o)			static_cast<t> (o)
    #define REINTERPRET_CAST(t, o)		reinterpret_cast<t> (o)
#else
    #define STATIC_CAST(t, o)			((t)(o))
    #define REINTERPRET_CAST(t, o)		(*(t*)(void*)&(o))
#endif

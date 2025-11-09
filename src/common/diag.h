// ----------------------------------------------------------------------------
//	diag.h - SDL2 version (stub)
//	Copyright (C) cisc 1999.
//	SDL2 port by Claude Code 2025
// ----------------------------------------------------------------------------
//	Diagnostic logging - disabled for SDL2 release builds

#pragma once

// Stub version: all logging disabled (variadic macros for flexibility)
#define LOG0(...)		((void)0)
#define LOG1(...)		((void)0)
#define LOG2(...)		((void)0)
#define LOG3(...)		((void)0)
#define LOG4(...)		((void)0)
#define LOG5(...)		((void)0)
#define LOG6(...)		((void)0)
#define LOG7(...)		((void)0)
#define LOG8(...)		((void)0)
#define LOG9(...)		((void)0)
#define DIAGINIT(...)	((void)0)
#define Log 0 ? 0 : printf

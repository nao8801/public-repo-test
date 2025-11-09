// ----------------------------------------------------------------------------
//	diag.h - SDL2 version (stub)
//	Copyright (C) cisc 1999.
//	SDL2 port by Claude Code 2025
// ----------------------------------------------------------------------------
//	Diagnostic logging - disabled for SDL2 release builds

#pragma once

// Stub version: all logging disabled
#define LOG0(m)		void (0)
#define LOG1(m,a)	void (0)
#define LOG2(m,a,b)	void (0)
#define LOG3(m,a,b,c)	void (0)
#define LOG4(m,a,b,c,d)	void (0)
#define LOG5(m,a,b,c,d,e)	void (0)
#define LOG6(m,a,b,c,d,e,f)	void (0)
#define LOG7(m,a,b,c,d,e,f,g)	void (0)
#define LOG8(m,a,b,c,d,e,f,g,h)	void (0)
#define LOG9(m,a,b,c,d,e,f,g,h,i)	void (0)
#define DIAGINIT(z)
#define Log 0 ? 0 : printf

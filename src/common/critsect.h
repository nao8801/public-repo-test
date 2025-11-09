// ----------------------------------------------------------------------------
//	critsect.h - SDL2 version (stub)
//	Copyright (C) cisc 1999.
//	SDL2 port by Claude Code 2025
// ----------------------------------------------------------------------------
//	Critical Section (thread synchronization) - stub version
//
//	Note: This is a stub implementation for initial build.
//	Future: Replace with pthread mutex for Linux, or keep as-is if
//	        single-threaded operation is acceptable.

#pragma once

// ---------------------------------------------------------------------------
//	CriticalSection - スレッド排他制御（スタブ版）
//
//	現在の実装: 何もしない（シングルスレッド前提）
//	将来の実装: pthread_mutex_t (Linux) または CRITICAL_SECTION (Windows)
//
class CriticalSection
{
public:
	CriticalSection() {}
	~CriticalSection() {}

	void EnterCriticalSection() {}  // 何もしない
	void LeaveCriticalSection() {}  // 何もしない

	// RAII pattern lock class (nested class)
	class Lock
	{
	public:
		Lock(CriticalSection& c) : cs(c) { cs.EnterCriticalSection(); }
		~Lock() { cs.LeaveCriticalSection(); }

	private:
		CriticalSection& cs;
	};
};

// ---------------------------------------------------------------------------
//	CSLock - 別名（互換性のため）
//
typedef CriticalSection::Lock CSLock;

// ---------------------------------------------------------------------------
//	status.h - SDL2 version (stub)
//	Copyright (C) cisc 1998, 1999.
//	SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//	Status Display (status bar) - stub version
//
//	Note: This is a stub implementation for initial build.
//	Future: Replace with SDL2-based status display.
//	        Current stub does nothing (no status bar shown).

#pragma once

#include "types.h"

// ---------------------------------------------------------------------------
//	StatusDisplay - ステータス表示クラス（スタブ版）
//
//	現在の実装: 何もしない（ステータスバーなし）
//	将来の実装: SDL2のテキスト描画でステータスバーを表示
//
class StatusDisplay
{
public:
	StatusDisplay() {}
	~StatusDisplay() {}

	// 全ての操作はスタブ（何もしない）
	bool Init(void* hwndparent) { return true; }
	void Cleanup() {}

	bool Enable(bool sfs=false) { return true; }
	bool Disable() { return true; }
	int GetHeight() { return 0; }
	void DrawItem(void* dis) {}
	void FDAccess(uint dr, bool hd, bool active) {}
	void UpdateDisplay() {}
	void WaitSubSys() {}

	bool Show(int priority, int duration, const char* msg, ...) { return true; }
	void Update() {}
	uint GetTimerID() { return 0; }

	void* GetHWnd() { return nullptr; }
};

// グローバルインスタンス（スタブ）
extern StatusDisplay statusdisplay;

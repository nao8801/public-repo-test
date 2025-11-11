// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1997, 2001.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  SoundDumpPipe - WAV録音機能
//  Win32版のSoundDumpPipeをSDL2版に移植

#pragma once

#include "common/types.h"
#include "common/soundsrc.h"
#include "common/critsect.h"
#include <string>

// ---------------------------------------------------------------------------
//  SoundDumpPipe - SoundSourceのラッパーで、音声データをWAVファイルに記録
//
class SoundDumpPipe : public SoundSource
{
public:
	SoundDumpPipe();
	~SoundDumpPipe();

	void SetSource(SoundSource* source) 
	{ 
		source_ = source; 
	}
	
	ulong GetRate()
	{
		return source_ ? source_->GetRate() : 0;
	}
	
	int GetChannels()
	{
		return source_ ? source_->GetChannels() : 0;
	}
	
	int Get(Sample* dest, int samples);
	
	int GetAvail()
	{
		return INT_MAX;
	}

	bool DumpStart(const char* filename);
	bool DumpStop();
	bool IsDumping() { return dumpstate_ != IDLE; }

private:
	enum DumpState
	{
		IDLE, STANDBY, DUMPING
	};

	void Dump(Sample* dest, int samples);

	SoundSource* source_;
	std::string dumpfile_;

	FILE* fp_;					// WAVファイルハンドル
	uint32_t data_size_pos_;	// dataチャンクのサイズ位置（後で更新）
	
	DumpState dumpstate_;
	int dumpedsample_;
	ulong dumprate_;

	CriticalSection cs_;
};


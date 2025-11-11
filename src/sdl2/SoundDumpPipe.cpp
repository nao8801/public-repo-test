// ---------------------------------------------------------------------------
//  M88 - PC-8801 Emulator (SDL2 version)
//  Copyright (C) cisc 1997, 2001.
//  SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//  SoundDumpPipe 実装
//  Win32版のSoundDumpPipeをSDL2版に移植（mmio APIを標準ファイルI/Oに置き換え）

#include "SoundDumpPipe.h"
#include "common/misc.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  構築・破棄
//
SoundDumpPipe::SoundDumpPipe()
	: source_(0)
	, dumpstate_(IDLE)
	, fp_(0)
	, data_size_pos_(0)
	, dumpedsample_(0)
	, dumprate_(0)
{
}

SoundDumpPipe::~SoundDumpPipe()
{
	DumpStop();
}

// ---------------------------------------------------------------------------
//  ダンプ開始
//
bool SoundDumpPipe::DumpStart(const char* filename)
{
	if (fp_)
		return false;

	dumpfile_ = filename;

	// WAVファイルを開く
	fp_ = fopen(filename, "wb");
	if (!fp_)
		return false;

	// RIFFヘッダー
	fwrite("RIFF", 1, 4, fp_);
	uint32_t file_size = 0;  // 後で更新
	fwrite(&file_size, 4, 1, fp_);
	fwrite("WAVE", 1, 4, fp_);

	// fmtチャンク
	fwrite("fmt ", 1, 4, fp_);
	uint32_t fmt_size = 16;
	fwrite(&fmt_size, 4, 1, fp_);
	
	dumprate_ = GetRate();
	
	uint16_t audio_format = 1;  // PCM
	fwrite(&audio_format, 2, 1, fp_);
	uint16_t num_channels = GetChannels();
	fwrite(&num_channels, 2, 1, fp_);
	fwrite(&dumprate_, 4, 1, fp_);
	uint32_t byte_rate = num_channels * dumprate_ * 2;  // 16bit
	fwrite(&byte_rate, 4, 1, fp_);
	uint16_t block_align = num_channels * 2;
	fwrite(&block_align, 2, 1, fp_);
	uint16_t bits_per_sample = 16;
	fwrite(&bits_per_sample, 2, 1, fp_);

	// dataチャンク
	fwrite("data", 1, 4, fp_);
	data_size_pos_ = ftell(fp_);
	uint32_t data_size = 0;  // 後で更新
	fwrite(&data_size, 4, 1, fp_);

	dumpstate_ = STANDBY;
	dumpedsample_ = 0;
	
	printf("[SoundDumpPipe] Recording started: %s\n", filename);
	return true;
}

// ---------------------------------------------------------------------------
//  ダンプ終了
//
bool SoundDumpPipe::DumpStop()
{
	if (dumpstate_ != IDLE)
	{
		CriticalSection::Lock lock(cs_);
		
		if (fp_)
		{
			// dataチャンクのサイズを更新
			long current_pos = ftell(fp_);
			uint32_t data_size = current_pos - data_size_pos_ - 4;
			fseek(fp_, data_size_pos_, SEEK_SET);
			fwrite(&data_size, 4, 1, fp_);
			
			// RIFFチャンクのサイズを更新
			uint32_t file_size = current_pos - 8;
			fseek(fp_, 4, SEEK_SET);
			fwrite(&file_size, 4, 1, fp_);
			
			fclose(fp_);
			fp_ = 0;
		}
		
		int curtime = dumpedsample_ / dumprate_;
		printf("[SoundDumpPipe] Recording stopped: %s [%02d:%02d]\n", 
		       dumpfile_.c_str(), curtime/60, curtime%60);
		
		dumpstate_ = IDLE;
	}
	return true;
}

// ---------------------------------------------------------------------------
//  音源からデータを取得（同時にWAVファイルに書き込む）
//
int SoundDumpPipe::Get(Sample* dest, int samples)
{
	if (!source_)
		return 0;

	if (dumpstate_ == IDLE)
		return source_->Get(dest, samples);

	int avail = source_->GetAvail();
	
	int actual_samples = source_->Get(dest, Min(avail, samples));

	int nch = GetChannels();
	memset(dest + actual_samples * nch, 0, (samples - actual_samples) * nch * sizeof(Sample));

	CriticalSection::Lock lock(cs_);
	if (dumpstate_ != IDLE)
	{
		Dump(dest, actual_samples);
	}

	return actual_samples;
}

// ---------------------------------------------------------------------------
//  WAVファイルにデータを書き込む
//
void SoundDumpPipe::Dump(Sample* dest, int samples)
{
	if (!fp_)
		return;
		
	int nch = GetChannels();

	// 冒頭の無音部をカットする
	if (dumpstate_ == STANDBY)
	{
		int i;
		uint32_t* s = (uint32_t*) dest;
		for (i=0; i<samples && *s == 0; i++, s++)
			;
		dest += i * nch;
		samples -= i;
		if (samples > 0)
		{
			dumpstate_ = DUMPING;
			printf("[SoundDumpPipe] Recording started (silence skipped: %d samples)\n", i);
		}
	}

	if (samples > 0)
	{
		fwrite(dest, sizeof(Sample), samples * nch, fp_);

		// 録音時間表示（1秒ごと）
		int prevtime = dumpedsample_ / dumprate_;
		dumpedsample_ += samples;
		int curtime = dumpedsample_ / dumprate_;
		// 進捗の定期出力は抑止（デバッグ時のみ有効化）
	}
}


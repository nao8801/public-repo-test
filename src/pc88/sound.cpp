// ---------------------------------------------------------------------------
//	M88 - PC-8801 Emulator
//	Copyright (C) cisc 1997, 2001.
// ---------------------------------------------------------------------------
//	$Id: sound.cpp,v 1.32 2003/05/19 01:10:32 cisc Exp $

#include "headers.h"
#include "types.h"
#include "misc.h"
#include "pc88/sound.h"
#include "pc88/pc88.h"
#include "pc88/config.h"

//#define LOGNAME "sound"
#include "diag.h"

using namespace PC8801;

// ---------------------------------------------------------------------------
//	生成・破棄
//
Sound::Sound()
: Device(0), sslist(0), mixingbuf(0), enabled(false), cfgflg(0), accumulated_time(0)
{
}

Sound::~Sound()
{
	Cleanup();
}

// ---------------------------------------------------------------------------
//	初期化とか
//
bool Sound::Init(PC88* pc88, uint rate, int bufsize)
{
	pc = pc88;
	prevtime = pc->GetCPUTick();
	enabled = false;
	mixthreshold = 16;
	accumulated_time = 0;
	
	if (!SetRate(rate, bufsize))
		return false;
	
	// 時間カウンタが一周しないように定期的に更新する
	pc88->AddEvent(5000, this, STATIC_CAST(TimeFunc, &Sound::UpdateCounter), 0, true);
	return true;
}

// ---------------------------------------------------------------------------
//	レート設定
//	clock:		OPN に与えるクロック
//	bufsize:	バッファ長 (サンプル単位?)
//
bool Sound::SetRate(uint rate, int bufsize)
{
	mixrate = 55467;

	// 各音源のレート設定を変更
	for (SSNode* n = sslist; n; n = n->next)
		n->ss->SetRate(mixrate);
	
	enabled = false;
	
	// 古いバッファを削除
	soundbuf.Cleanup();
	delete[] mixingbuf;	mixingbuf = 0;

	// 新しいバッファを用意
	samplingrate = rate;
	buffersize = bufsize;
	if (bufsize > 0)
	{
//		if (!soundbuf.Init(this, bufsize))
//			return false;
		if (!soundbuf.Init(this, bufsize, rate))
			return false;

		mixingbuf = new int32[2 * bufsize];
		if (!mixingbuf)
			return false;

		rate50 = mixrate / 50;
		tdiff = 0;
		enabled = true;
	}
	return true;
}

// ---------------------------------------------------------------------------
//	後片付け
//
void Sound::Cleanup()
{
	// 各音源を切り離す。(音源自体の削除は行わない)
	for (SSNode* n = sslist; n; )
	{
		SSNode* next = n->next;
		delete[] n;
		n = next;
	}
	sslist = 0;

	// バッファを開放
	soundbuf.Cleanup();
	delete[] mixingbuf; mixingbuf = 0;
}

// ---------------------------------------------------------------------------
//	音合成
//
int Sound::Get(Sample* dest, int nsamples)
{
	int mixsamples = Min(nsamples, buffersize);
	if (mixsamples > 0)
	{
		// 合成
		{
			memset(mixingbuf, 0, mixsamples * 2 * sizeof(int32));
			CriticalSection::Lock lock(cs_ss);
			for (SSNode* s = sslist; s; s = s->next)
				s->ss->Mix(mixingbuf, mixsamples);
		}

		int32* src = mixingbuf;
		for (int n = mixsamples; n>0; n--)
		{
			int32 l = *src++ >> 1;  // -6 dB attenuation
			int32 r = *src++ >> 1;  // -6 dB attenuation
			*dest++ = Limit(l, 32767, -32768);
			*dest++ = Limit(r, 32767, -32768);
		}
	}
	return mixsamples;
}

// ---------------------------------------------------------------------------
//	音合成
//
int Sound::Get(SampleL* dest, int nsamples)
{
	// 合成
	memset(dest, 0, nsamples * 2 * sizeof(int32));
	CriticalSection::Lock lock(cs_ss);
	for (SSNode* s = sslist; s; s = s->next)
		s->ss->Mix(dest, nsamples);

	// -6 dB attenuation after mixing to avoid clipping
	int total = nsamples * 2;
	for (int i = 0; i < total; i++) {
		dest[i] >>= 1;
	}
	return nsamples;
}


// ---------------------------------------------------------------------------
//	設定更新
//
void Sound::ApplyConfig(const Config* config)
{
	mixthreshold = (config->flags & Config::precisemixing) ? 100 : 2000;
}

// ---------------------------------------------------------------------------
//	音源を追加する
//	Sound が持つ音源リストに，ss で指定された音源を追加，
//	ss の SetRate を呼び出す．
//
//	arg:	ss		追加する音源 (ISoundSource)
//	ret:	S_OK, E_FAIL, E_OUTOFMEMORY
//
bool Sound::Connect(ISoundSource* ss)
{
	CriticalSection::Lock lock(cs_ss);

	// 音源は既に登録済みか？;
	SSNode** n;
	for (n = &sslist; *n; n=&((*n)->next))
	{
		if ((*n)->ss == ss)
			return false;
	}
	
	SSNode* nn = new SSNode;
	if (nn)
	{
		*n = nn;
		nn->next = 0;
		nn->ss = ss;
		ss->SetRate(mixrate);
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
//	音源リストから指定された音源を削除する
//
//	arg:	ss		削除する音源
//	ret:	S_OK, E_HANDLE
//
bool Sound::Disconnect(ISoundSource* ss)
{
	CriticalSection::Lock lock(cs_ss);
	
	for (SSNode** r = &sslist; *r; r=&((*r)->next))
	{
		if ((*r)->ss == ss)
		{
			SSNode* d = *r;
			*r = d->next;
			delete d;
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
//	更新処理
//	(指定された)音源の Mix を呼び出し，現在の時間まで更新する	
//	音源の内部状態が変わり，音が変化する直前の段階で呼び出すと
//	精度の高い音再現が可能になる(かも)．
//
//	arg:	src		更新する音源を指定(今の実装では無視されます)
//
bool Sound::Update(ISoundSource* /*src*/)
{
	uint32 currenttime = pc->GetCPUTick();
	
	uint32 time = currenttime - prevtime;
	if (!enabled)
		return true;
	
	// 無音時でもバッファを一定に保つため、timeが小さくても累積的に処理する
	// 1フレーム分（60 FPS = 1667 ticks）を最小単位として処理
	const uint32 min_frame_ticks = 1667;  // 1フレーム = 16.67ms @ 4MHz
	
	if (time > mixthreshold)
	{
		// 通常処理：timeが大きい場合は即座に処理
		prevtime = currenttime;
		// nsamples = 経過時間(s) * サンプリングレート
		// sample = ticks * rate / clock / 100000
		// sample = ticks * (rate/50) / clock / 2000

		// MulDiv(a, b, c) = (int64) a * b / c 
		int a = MulDiv(time, rate50, pc->GetEffectiveSpeed()) + tdiff;
//		a = MulDiv(a, mixrate, samplingrate);
		int samples = a / 2000;
		tdiff = a % 2000;
		
		soundbuf.Fill(samples);
	}
	else if (time > 0)
	{
		// timeが小さい場合でも、累積的に処理
		// 累積時間が1フレーム分を超えたら処理する
		accumulated_time += time;
		prevtime = currenttime;  // prevtimeは常に更新（次回のtime計算のため）
		
		if (accumulated_time >= min_frame_ticks)
		{
			// 累積時間分のサンプルを生成
			int a = MulDiv(accumulated_time, rate50, pc->GetEffectiveSpeed()) + tdiff;
			int samples = a / 2000;
			tdiff = a % 2000;
			
			soundbuf.Fill(samples);
			accumulated_time = 0;
		}
		else
		{
			// 累積時間がまだ足りない場合は待つ
			// ただし、長時間待ちすぎないように、一定時間経過したら強制的に処理
			if (accumulated_time > 0 && accumulated_time > 5000)
			{
				// 5ms以上経過した場合は強制的に処理（バッファ枯渇防止）
				int a = MulDiv(accumulated_time, rate50, pc->GetEffectiveSpeed()) + tdiff;
				int samples = a / 2000;
				tdiff = a % 2000;
				
				soundbuf.Fill(samples);
				accumulated_time = 0;
			}
		}
	}
	return true;
}

// ---------------------------------------------------------------------------
//	今まで合成された時間の，1サンプル未満の端数(0-1999)を求める
//
int IFCALL Sound::GetSubsampleTime(ISoundSource* /*src*/)
{
	return tdiff;
}

// ---------------------------------------------------------------------------
//	リセット時に内部状態をクリア
//
void Sound::Reset()
{
	if (pc) {
		prevtime = pc->GetCPUTick();
	}
	tdiff = 0;
	accumulated_time = 0;
}

// ---------------------------------------------------------------------------
//	定期的に内部カウンタを更新
//
void IOCALL Sound::UpdateCounter(uint)
{
	if ((pc->GetCPUTick() - prevtime) > 40000)
	{
		Update(0);
	}
}

// ---------------------------------------------------------------------------
//	file.h - SDL2 version (stub)
//	Copyright (C) cisc 1998, 1999.
//	SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//	Generic file I/O class - stub version
//
//	Note: This is a stub implementation for initial build.
//	Future: Replace with proper SDL2/POSIX file I/O implementation.
//	        Current stub always fails to open files (rhythm samples won't load).

#pragma once

#include "types.h"

// ---------------------------------------------------------------------------
//	FileIO - ファイル入出力クラス（スタブ版）
//
//	現在の実装: 全ての操作が失敗する（リズム音源サンプルは読み込まれない）
//	将来の実装: FILE* (POSIX) または std::fstream
//
class FileIO
{
public:
	enum Flags
	{
		open		= 0x000001,
		readonly	= 0x000002,
		create		= 0x000004,
	};

	enum SeekMethod
	{
		begin = 0, current = 1, end = 2,
	};

	enum Error
	{
		success = 0,
		file_not_found,
		sharing_violation,
		unknown = -1
	};

public:
	FileIO() : flags(0), lorigin(0), error(file_not_found) {}
	FileIO(const char* filename, uint flg = 0) : flags(0), lorigin(0), error(file_not_found) {}
	virtual ~FileIO() {}

	// 全ての操作はスタブ（何もしない）
	bool Open(const char* filename, uint flg = 0) { error = file_not_found; return false; }
	bool CreateNew(const char* filename) { error = unknown; return false; }
	bool Reopen(uint flg = 0) { return false; }
	void Close() {}
	Error GetError() { return error; }

	int32 Read(void* dest, int32 len) { return 0; }
	int32 Write(const void* src, int32 len) { return 0; }
	bool Seek(int32 fpos, SeekMethod method) { return false; }
	int32 Tellp() { return 0; }
	bool SetEndOfFile() { return false; }

	uint GetFlags() { return flags; }
	void SetLogicalOrigin(int32 origin) { lorigin = origin; }

private:
	uint flags;
	uint32 lorigin;
	Error error;

	FileIO(const FileIO&);
	const FileIO& operator=(const FileIO&);
};

// ---------------------------------------------------------------------------
//	FileFinder - ファイル検索クラス（スタブ版）
//
class FileFinder
{
public:
	FileFinder() {}
	~FileFinder() {}

	bool FindFile(char* szSearch) { return false; }
	bool FindNext() { return false; }

	const char* GetFileName() { return ""; }
	uint32 GetFileAttr() { return 0; }
	const char* GetAltName() { return ""; }
};

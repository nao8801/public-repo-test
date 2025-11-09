// ---------------------------------------------------------------------------
//	file.h - SDL2/POSIX version
//	Copyright (C) cisc 1998, 1999.
//	SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//	Generic file I/O class - POSIX implementation

#pragma once

#include "types.h"
#include <cstdio>
#include <string>

// ---------------------------------------------------------------------------
//	FileIO - ファイル入出力クラス（POSIX版）
//
//	機能:
//	- POSIX標準のfopen/fread/fwrite/fseek/ftellを使用
//	- 大文字小文字を区別しないファイル検索（Linux/macOS対応）
//	- Windows互換インターフェース
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
	FileIO();
	FileIO(const char* filename, uint flg = 0);
	virtual ~FileIO();

	bool Open(const char* filename, uint flg = 0);
	bool CreateNew(const char* filename);
	bool Reopen(uint flg = 0);
	void Close();
	Error GetError() { return error; }

	int32 Read(void* dest, int32 len);
	int32 Write(const void* src, int32 len);
	bool Seek(int32 fpos, SeekMethod method);
	int32 Tellp();
	bool SetEndOfFile();

	uint GetFlags() { return flags; }
	void SetLogicalOrigin(int32 origin) { lorigin = origin; }

private:
	FILE* fp;
	uint flags;
	uint32 lorigin;
	Error error;
	std::string path;

	// 大文字小文字を区別しないファイル検索（Linux/macOS用）
	static std::string FindFileIgnoreCase(const char* filename);

	FileIO(const FileIO&);
	const FileIO& operator=(const FileIO&);
};

// ---------------------------------------------------------------------------
//	FileFinder - ファイル検索クラス（スタブ版）
//
//	注: 現在は使用されていないため、スタブのまま
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

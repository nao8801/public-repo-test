// ---------------------------------------------------------------------------
//	file.cpp - SDL2/POSIX version
//	Copyright (C) cisc 1998, 1999.
//	SDL2 port by Claude Code 2025
// ---------------------------------------------------------------------------
//	Generic file I/O class - POSIX implementation

#include "file.h"
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// ---------------------------------------------------------------------------
//	大文字小文字を区別しないファイル検索
//
//	引数:
//	  filename - 検索するファイル名（例: "pc88.rom"）
//
//	戻り値:
//	  見つかった場合: 実際のファイル名（例: "PC88.ROM"）
//	  見つからない場合: 元のファイル名をそのまま返す
//
std::string FileIO::FindFileIgnoreCase(const char* filename)
{
	if (!filename || !*filename)
		return "";

	// パスとファイル名を分離
	std::string fullpath = filename;
	std::string dirname = ".";
	std::string basename = filename;

	size_t lastslash = fullpath.find_last_of("/\\");
	if (lastslash != std::string::npos) {
		dirname = fullpath.substr(0, lastslash);
		basename = fullpath.substr(lastslash + 1);
	}

	// まず、指定されたファイル名をそのまま試す（高速パス）
	struct stat st;
	if (stat(filename, &st) == 0)
		return filename;

	// ディレクトリを開く
	DIR* dir = opendir(dirname.c_str());
	if (!dir)
		return filename;  // ディレクトリが開けない場合は元のファイル名を返す

	// ディレクトリ内のファイルを列挙して、大文字小文字を無視して比較
	struct dirent* entry;
	std::string result = filename;

	while ((entry = readdir(dir)) != nullptr) {
		if (strcasecmp(entry->d_name, basename.c_str()) == 0) {
			// 見つかった！実際のファイル名を構築
			if (dirname == ".") {
				result = entry->d_name;
			} else {
				result = dirname + "/" + entry->d_name;
			}
			break;
		}
	}

	closedir(dir);
	return result;
}

// ---------------------------------------------------------------------------
//	コンストラクタ
//
FileIO::FileIO()
	: fp(nullptr)
	, flags(0)
	, lorigin(0)
	, error(success)
{
}

FileIO::FileIO(const char* filename, uint flg)
	: fp(nullptr)
	, flags(0)
	, lorigin(0)
	, error(success)
{
	Open(filename, flg);
}

// ---------------------------------------------------------------------------
//	デストラクタ
//
FileIO::~FileIO()
{
	Close();
}

// ---------------------------------------------------------------------------
//	ファイルを開く
//
bool FileIO::Open(const char* filename, uint flg)
{
	Close();

	if (!filename || !*filename) {
		error = file_not_found;
		return false;
	}

	// 大文字小文字を区別しない検索
	std::string actualpath = FindFileIgnoreCase(filename);
	path = actualpath;

	// フラグに応じてモードを決定
	const char* mode;
	if (flg & create) {
		mode = "wb+";  // 新規作成（読み書き）
	} else if (flg & readonly) {
		mode = "rb";   // 読み取り専用
	} else {
		mode = "rb+";  // 読み書き
	}

	fp = fopen(actualpath.c_str(), mode);
	if (!fp) {
		error = file_not_found;
		return false;
	}

	flags = flg | open;
	error = success;
	return true;
}

// ---------------------------------------------------------------------------
//	新規ファイル作成
//
bool FileIO::CreateNew(const char* filename)
{
	return Open(filename, create);
}

// ---------------------------------------------------------------------------
//	ファイルを再オープン
//
bool FileIO::Reopen(uint flg)
{
	if (path.empty())
		return false;

	return Open(path.c_str(), flg);
}

// ---------------------------------------------------------------------------
//	ファイルを閉じる
//
void FileIO::Close()
{
	if (fp) {
		fclose(fp);
		fp = nullptr;
	}
	flags = 0;
}

// ---------------------------------------------------------------------------
//	読み込み
//
int32 FileIO::Read(void* dest, int32 len)
{
	if (!fp || len <= 0)
		return 0;

	size_t result = fread(dest, 1, len, fp);
	return static_cast<int32>(result);
}

// ---------------------------------------------------------------------------
//	書き込み
//
int32 FileIO::Write(const void* src, int32 len)
{
	if (!fp || len <= 0)
		return 0;

	size_t result = fwrite(src, 1, len, fp);
	return static_cast<int32>(result);
}

// ---------------------------------------------------------------------------
//	シーク
//
bool FileIO::Seek(int32 fpos, SeekMethod method)
{
	if (!fp)
		return false;

	int whence;
	switch (method) {
		case begin:   whence = SEEK_SET; break;
		case current: whence = SEEK_CUR; break;
		case end:     whence = SEEK_END; break;
		default:      return false;
	}

	// lorigin（論理オリジン）を考慮
	int32 pos = (method == begin) ? (fpos + lorigin) : fpos;

	return fseek(fp, pos, whence) == 0;
}

// ---------------------------------------------------------------------------
//	現在位置取得
//
int32 FileIO::Tellp()
{
	if (!fp)
		return 0;

	long pos = ftell(fp);
	if (pos < 0)
		return 0;

	// lorigin（論理オリジン）を考慮
	return static_cast<int32>(pos - lorigin);
}

// ---------------------------------------------------------------------------
//	ファイル終端設定
//
bool FileIO::SetEndOfFile()
{
	if (!fp)
		return false;

	// ftruncate は POSIX 標準
	int fd = fileno(fp);
	if (fd < 0)
		return false;

	long pos = ftell(fp);
	if (pos < 0)
		return false;

	return ftruncate(fd, pos) == 0;
}

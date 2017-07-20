#pragma once
#include <tchar.h>
#include <io.h>
#include <string>
#include <map>
using namespace std;

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

extern CHAR g_baseDir[MAX_PATH];

struct PDUSMS {
	UINT nSize;
	string sPDU;
};

class CUtils
{
public:
	CUtils();
	~CUtils();

public:
	static void Log(TCHAR *format, ...);
	static string& Replace_All(string& src, const string& old_value, const string& new_value);
	static size_t Find(const string &src, const string &tar);
	static int Split(map<string, string>& dest, const string src, string sep);
	static BOOL FileExits(string filepath);
};


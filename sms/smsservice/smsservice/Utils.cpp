#include "stdafx.h"
#include "Utils.h"

CHAR g_baseDir[MAX_PATH] = {0};

CUtils::CUtils() {}

CUtils::~CUtils() {}

void CUtils::Log(TCHAR *format, ...)
{
	INT nSize = 0;

	//声明一个指针，保存可变参数
	va_list argv;
	//argv指向第一个参数
	//遍历可变参数表
	//计算所需数组的大小
	va_start(argv, format);
	nSize = _vsntprintf(NULL, 0, format, argv);
	va_end(argv);

	TCHAR *tszMsg = new TCHAR[nSize + 2];
	memset(tszMsg, 0, sizeof(TCHAR)*(nSize + 2));
	//遍历并格式化数据
	va_start(argv, format);
	_vsntprintf(tszMsg, nSize, format, argv);
	va_end(argv);

	SYSTEMTIME tm;
	GetLocalTime(&tm);

	TCHAR csFile[MAX_PATH] = _T("0");
	_stprintf_s(csFile, _T("%Slog\\%04d%02d%02d.log"), g_baseDir, tm.wYear, tm.wMonth, tm.wDay);

	FILE *fp = NULL;
	_tfopen_s(&fp, csFile, _T("a"));
	if (NULL == fp)
		return;

	int nLen = lstrlen(tszMsg);
	setlocale(LC_ALL, "chs");// 设置中文环境
	if (nLen > 0 && tszMsg[nLen - 1] == _T('\n'))
	{
		tszMsg[nLen - 1] = 0;
		_ftprintf_s(fp, _T("%04d-%02d-%02d %02d:%02d:%02d %s"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tszMsg);
	}
	else
		_ftprintf_s(fp, _T("%04d-%02d-%02d %02d:%02d:%02d %s\n"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tszMsg);
	setlocale(LC_ALL, "C");// 还原

	delete[] tszMsg;
	tszMsg = NULL;
	fclose(fp);
}

string& CUtils::Replace_All(string& src, const string& old_value, const string& new_value)
{
	while (true)
	{
		size_t pos = src.find(old_value);
		if (pos != string::npos)
			src.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return src;
}

size_t CUtils::Find(const string& src, const string &tar)
{
	size_t pos(0);
	pos = src.find(tar);
	return pos;
}

int CUtils::Split(map<string, string>& dest, const string src, string sep)
{
	string tmp = src;
	size_t pos = tmp.find(sep);
	int i = 0;//索引
	string strKey, strVal;

	while (pos != string::npos)
	{
		//非空
		if (0 != pos)
		{
			strVal = tmp.substr(0, pos);
			if (i == 0)
				dest.insert(map<string, string>::value_type("TYPE", strVal));
			else if (i == 1)
				dest.insert(map<string, string>::value_type("STCD", strVal));
			else if (i == 2)
				dest.insert(map<string, string>::value_type("TM", strVal));

			else if (i % 2 == 1)
				strKey = strVal;
			else if (i % 2 == 0)
				dest.insert(map<string, string>::value_type(strKey, strVal));
			i++;
		}
		tmp = tmp.substr(pos + 1);
		if (tmp.empty())
			break;
		pos = tmp.find(sep);
	}
	if (tmp.empty())
		dest.insert(map<string, string>::value_type("END", strVal));
	else {
		dest.insert(map<string, string>::value_type("END", tmp));
		i++;
	}
	return i;
}

BOOL CUtils::FileExits(string filepath)
{
	int nRes = _access(filepath.data(), 0);
	if (nRes == 0)
		return TRUE;
	return FALSE;
}

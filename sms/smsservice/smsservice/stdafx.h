// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <process.h>

#include <locale.h>


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#ifndef DEBUG
#pragma comment(lib, "Advapi32.lib")
#endif // !RELEASE

// WatchDog.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ServiceManager.h"

#include <locale.h>
#include <process.h>

#define MAX_SLEEP_TIME 5000

SERVICE_STATUS g_servState;
SERVICE_STATUS_HANDLE g_servStateHandle;
HANDLE g_hServStopEvent = NULL;
HANDLE g_hListenThread = INVALID_HANDLE_VALUE;

SC_HANDLE g_hSCManager = NULL;

BOOL g_bExit = false;	//线程退出标识

TCHAR g_servName[32] = _T("WatchDog");
CHAR g_servNameW[32] = { 0 };
CHAR g_baseDir[MAX_PATH] = { 0 };

VOID WINAPI ServMain(unsigned long argc, TCHAR*argv[]);
VOID WINAPI ServCtrlHandler(unsigned long);
VOID ServInit(unsigned long argc, TCHAR*argv[]);
VOID ReportServStatus(unsigned long, unsigned long, unsigned long);

UINT WINAPI ListenThread(LPVOID lpParam);
BOOL OpenListenThread();
BOOL CloseListenThread();

VOID Log(TCHAR *format, ...);

int main(int argc, char *argv[])
{
	//获取程序运行目录
	CHAR szFilePath[MAX_PATH + 1];
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
#ifdef _WINDOWS_
	(strrchr(szFilePath, _T('\\')))[1] = 0;
#else
	(strrchr(szFilePath, _T('//')))[1] = 0;
#endif
	strcpy(g_baseDir, szFilePath);
	strcat(szFilePath, "mall.ini");

	CServiceManager servManager;
	servManager.InitParam(szFilePath);

	if (argc == 1)
	{
		GetPrivateProfileStringA("SERVICE", "name", "SmServ", g_servNameW, sizeof(g_servNameW), szFilePath);
		g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (g_hSCManager == NULL) {
			Log(_T("Open SCManager failed, error code: %d"), GetLastError());
			return -1;
		}

		SERVICE_TABLE_ENTRY serviceTable[] =
		{
			{ g_servName, (LPSERVICE_MAIN_FUNCTION)ServMain },
			{ NULL, NULL }
		};

		if (!StartServiceCtrlDispatcher(serviceTable))
		{
			Log(_T("[ERROR]Register Service CtrlManager Failed, Error Code: %d"), GetLastError());
		}
	}
	else
	{
		if (strcmp(argv[1], "install") == 0)
		{
			servManager.InstallService();
		}
		else if (strcmp(argv[1], "uninstall") == 0)
		{
			servManager.UninstallService();
		}
	}

    return 0;
}

VOID WINAPI ServMain(unsigned long argc, TCHAR*argv[])
{
	Log(_T("[INFO]Register Service CtrlHandler Start..."));
	g_servStateHandle = RegisterServiceCtrlHandler(g_servName, ServCtrlHandler);
	if (NULL == g_servStateHandle)
	{
		Log(_T("[ERROR]Register Service CtrlHandler Failed, Error Code: %d"), GetLastError());
		return;
	}
	Log(_T("[INFO]Register Service CtrlHandler Success"));
	g_servState.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_servState.dwServiceSpecificExitCode = 0;

	ReportServStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	ServInit(argc, argv);
}

VOID WINAPI ServCtrlHandler(unsigned long dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		ReportServStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		g_bExit = TRUE;
		CloseListenThread();
		Sleep(100);
		SetEvent(g_hServStopEvent);
		Sleep(100);
		ReportServStatus(g_servState.dwCurrentState, NO_ERROR, 0);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		ReportServStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		g_bExit = TRUE;
		CloseListenThread();
		Sleep(100);
		SetEvent(g_hServStopEvent);
		Sleep(100);
		ReportServStatus(g_servState.dwCurrentState, NO_ERROR, 0);
		break;
	default:
		break;
	}
}

VOID ServInit(unsigned long argc, TCHAR* argv[])
{
	Log(_T("[INFO]Initialize Service"));
	g_hServStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == g_hServStopEvent)
	{
		ReportServStatus(SERVICE_STOPPED, NO_ERROR, 0);
		Log(_T("[ERROR]Service Event Creates Failed, Error Code: %d"), GetLastError());
		return;
	}
	//开启线程读取短信信息
	OpenListenThread();
	//报告状态
	ReportServStatus(SERVICE_RUNNING, NO_ERROR, 0);
	//Log(0, "[INFO]服务开始运行");
	while (TRUE)
	{
		WaitForSingleObject(g_hServStopEvent, INFINITE);
		ReportServStatus(SERVICE_STOPPED, NO_ERROR, 0);
		Log(_T("[INFO]Service Stopped"));
		break;
	}
}

VOID ReportServStatus(unsigned long dwCurrentState, unsigned long dwWin32ExitCode, unsigned long dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	g_servState.dwCurrentState = dwCurrentState;
	g_servState.dwWin32ExitCode = dwWin32ExitCode;
	g_servState.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		g_servState.dwControlsAccepted = 0;
	else
		g_servState.dwControlsAccepted = SERVICE_ACCEPT_STOP
		| SERVICE_ACCEPT_SHUTDOWN;

	if (dwCurrentState == SERVICE_RUNNING
		|| dwCurrentState == SERVICE_STOPPED)
		g_servState.dwCheckPoint = 0;
	else
		g_servState.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus(g_servStateHandle, &g_servState);
}

BOOL OpenListenThread()
{
	if (g_hListenThread != INVALID_HANDLE_VALUE)
	{
		Log(_T("[INFO]Listen Thread is running"));
		return false;
	}

	g_bExit = false;
	UINT threadId = 0;
	g_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, NULL, 0, &threadId);
	if (g_hListenThread == INVALID_HANDLE_VALUE)
	{
		Log(_T("[ERROR]Listen Thread Starts Failed: %d"), GetLastError());
		return false;
	}
	Log(_T("[INFO]Listen Thread Starts Success"));
	//设置线程的优先级，高于普通线程
	if (!SetThreadPriority(g_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		Log(_T("[ERROR]Listen Thread Priority Sets Failed: %d"), GetLastError());
		return false;
	}
	Log(_T("[INFO]Listen Thread Priority Sets Success"));
	return true;
}

BOOL CloseListenThread()
{
	if (g_hListenThread != INVALID_HANDLE_VALUE)
	{
		Log(_T("[INFO]Listen Thread is Closing..."));
		//通知线程退出
		g_bExit = true;
		//等待线程退出
		//SleepEx(100, false);
		WaitForSingleObject(g_hListenThread, 3000);
		//设置线程句柄无效
		CloseHandle(g_hListenThread);
		g_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT WINAPI ListenThread(LPVOID lpParam)
{
	bool bRes = false;
	SC_HANDLE hService = NULL;
	SERVICE_STATUS servState;
	while (!g_bExit)
	{
		if (!bRes)
		{
			hService = OpenServiceA(g_hSCManager, g_servNameW, SERVICE_ALL_ACCESS);
			if (NULL != hService)
				bRes = true;
			else
				Log(_T("[ERROR]Open Service:%S Failed, Error Code: %d"), g_servNameW, GetLastError());
		}
		if (bRes)
		{
			QueryServiceStatus(hService, &servState);
			if (servState.dwCurrentState == SERVICE_PAUSED
				|| servState.dwCurrentState == SERVICE_STOPPED)
			{
				if(StartService(hService, NULL, NULL))
					Log(_T("[ERROR]Start Service:%S Success"), g_servNameW);
				else
					Log(_T("[ERROR]Start Service:%S Failed, Error Code: %d"), g_servNameW, GetLastError());
			}
		}
		Sleep(MAX_SLEEP_TIME);
	}
	return 0;
}

void Log(TCHAR *format, ...)
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
	_stprintf_s(csFile, _T("%Swatchdog.log"), g_baseDir);

	FILE *fp = NULL;
	_tfopen_s(&fp, csFile, _T("a"));
	if (NULL == fp)
		return;

	int nLen = lstrlen(tszMsg);
	setlocale(LC_ALL, "chinese-simplified");// 设置中文环境
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

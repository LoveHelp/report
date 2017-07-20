// smsservice.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MallManager.h"
#include "ServiceManager.h"
#include "Utils.h"

#include <iostream>
#include <fstream>

#define MAX_SLEEP_TIME 3000

CMallManager g_manMall;
CServiceManager g_manServ;

SERVICE_STATUS g_servState;
SERVICE_STATUS_HANDLE g_servStateHandle;
HANDLE g_hServStopEvent = NULL;

BOOL g_bExit = false;	//线程退出标识

TCHAR g_servName[32] = _T("0");
TCHAR g_iniFile[] = _T("mall.ini");

VOID WINAPI ServMain(unsigned long argc, TCHAR*argv[]);
VOID WINAPI ServCtrlHandler(unsigned long);
VOID ServInit(unsigned long argc, TCHAR*argv[]);
VOID ServReportEvent(LPTSTR);
VOID ReportServStatus(unsigned long, unsigned long, unsigned long);

VOID RunService();
VOID GenerateConfigFile(string filepath);

INT InitComm();
INT InitMall();

VOID CloseListenThread();

INT main(int argc, char *argv[])
{
	//获取程序运行目录
	CHAR szFilePath[MAX_PATH + 1];
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
#ifdef _WINDOWS_
	(strrchr(szFilePath, _T('\\')))[1] = 0;	//删除文件名，只获得路径
#else
	(strrchr(szFilePath, _T('//')))[1] = 0;	//删除文件名，只获得路径
#endif
	//保存运行目录
	strcpy(g_baseDir, szFilePath);
	//配置文件
	strcat(szFilePath, "mall.ini");
	//生成默认配置文件
	GenerateConfigFile(szFilePath);

	//初始化猫管理实例
	g_manMall.InitParam(szFilePath);
	//初始化服务管理实例
	g_manServ.InitParam(szFilePath);
	
	if (argc == 1)
	{
		RunService();
	}
	else
	{
		if (strcmp(argv[1], "install") == 0)
		{
			std::wcout << _T("start install...") << std::endl;
			g_manServ.InstallService();
		}
		else if (strcmp(argv[1], "uninstall") == 0)
		{
			std::wcout << _T("start uninstall...\n");
			g_manServ.UninstallService();
		}
	}

    return 0;
}

VOID RunService()
{
	SERVINFO servInfo = g_manServ.GetServInfo();
	_tcscpy_s(g_servName, servInfo.name);
	CUtils::Log(_T("[INFO]Service Name: %s"), g_servName);
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ servInfo.name, (LPSERVICE_MAIN_FUNCTION)ServMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(serviceTable))
	{
		ServReportEvent(TEXT("注册服务管理器失败"));
		CUtils::Log(_T("[ERROR]Register Service CtrlManager Failed, Error Code: %d"), GetLastError());
	}
}

VOID WINAPI ServMain(unsigned long argc, TCHAR*argv[])
{
	CUtils::Log(_T("[INFO]Register Service CtrlHandler Start..."));
	g_servStateHandle = RegisterServiceCtrlHandler(g_servName, ServCtrlHandler);
	if (NULL == g_servStateHandle)
	{
		ServReportEvent(TEXT("注册服务状态控制程序失败"));
		CUtils::Log(_T("[ERROR]Register Service CtrlHandler Failed, Error Code: %d"), GetLastError());
		return;
	}
	CUtils::Log(_T("[INFO]Register Service CtrlHandler Success"));
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
		ServReportEvent(TEXT("服务停止运行"));
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		ReportServStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		g_bExit = TRUE;
		CloseListenThread();
		Sleep(100);
		SetEvent(g_hServStopEvent);
		Sleep(100);
		ReportServStatus(g_servState.dwCurrentState, NO_ERROR, 0);
		ServReportEvent(TEXT("系统关机了"));
		break;
	default:
		break;
	}
}

VOID ServInit(unsigned long argc, TCHAR* argv[])
{
	int nRes = 0;
	CUtils::Log(_T("[INFO]Initialize Service"));
	g_hServStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == g_hServStopEvent)
	{
		ReportServStatus(SERVICE_STOPPED, NO_ERROR, 0);
		CUtils::Log(_T("[ERROR]Service Event Creates Failed, Error Code: %d"), GetLastError());
		return;
	}
	nRes = InitComm();
	if (nRes != 0)
	{
		SetEvent(g_hServStopEvent);
	}
	else
	{
		nRes = InitMall();
		if (nRes != 0)
			SetEvent(g_hServStopEvent);
	}
	
	//报告状态
	ReportServStatus(SERVICE_RUNNING, NO_ERROR, 0);
	//Log(0, "[INFO]服务开始运行");
	while (TRUE)
	{
		WaitForSingleObject(g_hServStopEvent, INFINITE);
		ReportServStatus(SERVICE_STOPPED, NO_ERROR, 0);
		CUtils::Log(_T("[INFO]Service Stopped"));
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

INT InitComm()
{
	TCHAR scErr[1024] = _T("0");
	INT nErr = 0;
	BOOL bRes = FALSE;
	//串口开启
	bRes = g_manMall.OpenPort();
	if (!bRes)
	{
		memset(scErr, 0, sizeof(scErr));
		g_manMall.ErrorInfo(scErr, 1024);
		CUtils::Log(_T("[ERROR]SerialPort Opens Failed, Error Code: %d, Error Message: %s"), g_manMall.GetErrNo(), scErr);
		return 1;
	}
	CUtils::Log(_T("[INFO]SerialPort Opens Success"));
	//串口初始化
	int nRet = g_manMall.InitPort();
	if (nRet != 0)
	{
		if (nRet == 1)
			CUtils::Log(_T("[ERROR]SerialPort Initializes Failed, Error Message: SerialPort is Closed"));
		else
		{
			memset(scErr, 0, sizeof(scErr));
			g_manMall.ErrorInfo(scErr, 1024);
			CUtils::Log(_T("[ERROR]SerialPort Initializes Failed, Error Code: %d, Error Message: %s"), g_manMall.GetErrNo(), scErr);
		}

		return 2;
	}
	CUtils::Log(_T("[INFO]SerialPort Initializes Success"));
	return 0;
}

INT InitMall()
{
	TCHAR scErr[1024] = _T("0");
	INT nErr = 0;
	BOOL bRes = FALSE;
	
	//连接短信猫
	bRes = g_manMall.ConnectMall();
	if (!bRes)
	{
		memset(scErr, 0, sizeof(scErr));
		g_manMall.ErrorInfo(scErr, 1024);
		nErr = g_manMall.GetErrNo();
		if (nErr == 0)
			CUtils::Log(_T("[ERROR]GSM MODEM Connects Failed, Error Code: %d, Error Message: %s"), g_manMall.GetErrNo(), _T("Time Out"));
		else
			CUtils::Log(_T("[ERROR]GSM MODEM Connects Failed, Error Code: %d, Error Message: %s"), g_manMall.GetErrNo(), scErr);
		return 1;
	}
	CUtils::Log(_T("[INFO]GSM MODEM Connects/Initializes Success"));
	return 0;
}

VOID ServReportEvent(LPTSTR szFunction)
{
	HANDLE hEvent;
	LPCTSTR szString[2] = { 0 };
	TCHAR buffer[80] = { 0 };

	hEvent = RegisterEventSource(NULL, g_servName);
	if (NULL != hEvent)
	{
		StringCchPrintf(buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());
		szString[0] = g_servName;
		szString[1] = buffer;
		ReportEvent(hEvent,        // event log handle
			EVENTLOG_ERROR_TYPE, // event type
			0,                   // event category
			0,					// event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			szString,         // array of strings
			NULL);               // no binary data
		DeregisterEventSource(hEvent);
	}
}

VOID GenerateConfigFile(string filepath)
{
	BOOL bExists = CUtils::FileExits(filepath);
	string strSections;
	DWORD dwSize = 0;

	if (!bExists)
	{
		string szAppName = "COM";
		WritePrivateProfileStringA(szAppName.data(), "com", "1", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "baund", "115200", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "parity", "0", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "databit", "8", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "stopbit", "0", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "smc", "15893553324", filepath.data());

		szAppName = "SERVICE";
		WritePrivateProfileStringA(szAppName.data(), "name", "SmServ", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "desc", "SMS Service", filepath.data());

		/*szAppName = "MYSQL";
		WritePrivateProfileStringA(szAppName.data(), "addr", "localhost", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "user", "root", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "password", "s123", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "database", "waterbureau", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "port", "3306", filepath.data());*/

		szAppName = "SERVER";
		WritePrivateProfileStringA(szAppName.data(), "addr", "127.0.0.1", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "port", "80", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "url", "localhost", filepath.data());
		WritePrivateProfileStringA(szAppName.data(), "path", "/checkbx.php", filepath.data());
	}
}

VOID CloseListenThread()
{
	g_manMall.CloseWatchThread();
	g_manMall.CloseListenThread();
	g_manMall.CloseWriteThread();
}
#include "stdafx.h"
#include "ServiceManager.h"

#include <iostream>

CServiceManager::CServiceManager(VOID)
{
	memset(&m_servInfo, 0, sizeof(SERVINFO));
}

CServiceManager::~CServiceManager(){}

VOID CServiceManager::InitParam(char iniFile[])
{
	TCHAR scAppName[] = { _T("SERV") };
	TCHAR szFilePath[MAX_PATH] = { 0 };
	//default
	_stprintf_s(m_servInfo.name, _T("SmServ"));
	_stprintf_s(m_servInfo.desc, _T("SMS Service"));
	_stprintf_s(szFilePath, _T("%S"), iniFile);

	GetPrivateProfileString(scAppName, _T("name"), _T("SmServ"), m_servInfo.name, sizeof(m_servInfo.name), szFilePath);
	GetPrivateProfileString(scAppName, _T("desc"), _T("SMS Service"), m_servInfo.desc, sizeof(m_servInfo.desc), szFilePath);
}

SERVINFO CServiceManager::GetServInfo()
{
	return this->m_servInfo;
}

BOOL CServiceManager::InstallService() 
{
	SC_HANDLE hSCManager;
	SC_HANDLE hSCService;
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		std::cout << "Open SCManager failed, error code: " << GetLastError() << std::endl;
		return FALSE;
	}

	TCHAR szModuleFileName[MAX_PATH];
	GetModuleFileName(NULL, szModuleFileName, MAX_PATH);

	hSCService = CreateService(hSCManager,
		m_servInfo.name,
		m_servInfo.desc,
		SC_MANAGER_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		szModuleFileName,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	if (hSCManager == NULL) {
		return FALSE;
	}

	StartService(hSCService, NULL, NULL);
	std::cout << "Install service success." << std::endl;

	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);

	return TRUE;
}

BOOL CServiceManager::UninstallService() {
	SC_HANDLE hSCManager;
	SC_HANDLE hSCService;
	SERVICE_STATUS curStatus;
	SERVICE_STATUS ctrlstatus;

	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		std::cout << "Open SCManager failed, error code: " << GetLastError() << std::endl;
		return FALSE;
	}
	hSCService = OpenService(hSCManager, m_servInfo.name, SERVICE_ALL_ACCESS);
	if (hSCService == NULL) {
		std::cout << "Open service failed, error code: " << GetLastError() << std::endl;
		return FALSE;
	}
	if (!QueryServiceStatus(hSCService, &curStatus)) {
		std::cout << "Query service state failed, error code: " << GetLastError() << std::endl;
		return FALSE;
	}

	if (curStatus.dwCurrentState != SERVICE_STOPPED) {
		if (!ControlService(hSCService, SERVICE_CONTROL_STOP, &ctrlstatus)) {
			std::cout << "Stop service failed, error code: " << GetLastError() << std::endl;
			return FALSE;
		}
	}

	if (DeleteService(hSCService)) {
		std::cout << "Uninstall service success." << std::endl;
	}
	else {
		std::cout << "Uninstall service failed, error code: " << GetLastError() << std::endl;
	}

	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);

	return TRUE;
}
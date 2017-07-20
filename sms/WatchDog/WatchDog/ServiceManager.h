#pragma once

struct SERVINFO
{
	TCHAR name[32];
	TCHAR desc[256];
};

class CServiceManager
{
public:
	CServiceManager(VOID);
	~CServiceManager();

private:
	/** �����ļ� */
	TCHAR m_iniFile[MAX_PATH];
	SERVINFO m_servInfo;

public:
	VOID InitParam(char iniFile[]);
	SERVINFO GetServInfo();
	BOOL InstallService();
	BOOL UninstallService();
};


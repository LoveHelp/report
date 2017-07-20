#pragma once

//服务信息结构体
struct SERVINFO
{
	TCHAR name[32];		//服务名
	TCHAR desc[256];	//显示名
};

class CServiceManager
{
public:
	CServiceManager(VOID);
	~CServiceManager();

private:
	/** 配置文件 */
	TCHAR m_iniFile[MAX_PATH];
	SERVINFO m_servInfo;

public:
	//初始化服务参数
	VOID InitParam(char iniFile[]);
	//获取服务信息
	SERVINFO GetServInfo();
	//安装服务
	BOOL InstallService();
	//卸载服务
	BOOL UninstallService();
};


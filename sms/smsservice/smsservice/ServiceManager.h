#pragma once

//������Ϣ�ṹ��
struct SERVINFO
{
	TCHAR name[32];		//������
	TCHAR desc[256];	//��ʾ��
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
	//��ʼ���������
	VOID InitParam(char iniFile[]);
	//��ȡ������Ϣ
	SERVINFO GetServInfo();
	//��װ����
	BOOL InstallService();
	//ж�ط���
	BOOL UninstallService();
};


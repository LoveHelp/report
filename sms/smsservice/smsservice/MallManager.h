#pragma once

#include "Utils.h"
#include "SmsHelper.h"
//#include "MySQLHelper.h"
#include "HttpUtils.h"

#include <list>
using namespace std;

//������Ϣ�ṹ��
struct COMMINFO
{
	INT nComm;
	INT nBaund;
	INT nParity;
	INT nDatabit;
	INT nStopbit;
};

//������Ϣ�ṹ��
struct SMSINFO
{
	INT nCount;
	string strTxt;
	bool operator==(SMSINFO& value)
	{
		if (strTxt.compare(value.strTxt) == 0)
			return true;
		return false;
	}
};

class CMallManager
{
public:
	CMallManager(void);
	~CMallManager();

private:
	/** ���ھ�� */
	HANDLE m_hComm;
	/** ���ڼ����߳� */
	HANDLE m_hListenThread;
	/** ��Ϣ�����߳̾�� */
	HANDLE m_hWriteDatabaseThread;
	/** ����è״̬����߳̾�� */
	HANDLE m_hMallWatchThread;
	/** ���ڼ����߳��˳���־ */
	static bool m_bExitA;
	/** ���ݴ����߳��˳���־ */
	static bool m_bExitB;
	/** ����è״̬����߳��˳���־ */
	static bool m_bExitC;
	/** ���ڴ򿪱�־ */
	bool m_bOpen;
	/** ͬ������,�ٽ�������(����) */
	CRITICAL_SECTION m_csCommunicationSync;
	/** ������ */
	int m_errno;
	/** ������Ϣ */
	COMMINFO m_commInfo;
	/** �������ĺ��� */
	char m_addr[16];
	/** ���� */
	string m_cmd;
	/** ���Ž����� */
	CSmsHelper *m_pSmsHelper;
	/** Http������*/
	CHttpUtils *m_pHttpUtils;
	/** ͬ�����⣬�ٽ�������*/
	CRITICAL_SECTION m_csDataSync;
	/** ���������б� */
	list<SMSINFO> m_dataList;
	/** è��ʼ��״̬��-1Ĭ��ֵ 0Ϊ��ʼ�� 1��ʼ���ɹ� */
	int m_initState;

public:
	//��ȡ���ڳ�ʼ������
	void InitParam(char iniFile[]);
	//��⴮���Ƿ���
	BOOL IsOpened();
	//�򿪴���
	BOOL OpenPort();
	//��ʼ������
	BOOL InitPort();
	//�رմ���
	void ClosePort();
	//��ȡ������
	INT GetErrNo();
	//������Ϣ
	void ErrorInfo(TCHAR err[], INT nLen);
	//����è
	BOOL ConnectMall();
	//��ʼ��è
	int InitMall(string& strData);

private:
	//�򴮿ڷ���ָ��
	//strDataΪָ���ֵ
	BOOL WriteComm(string& strData);
	//��ȡ�������뻺�������ݳ���
	UINT GetBytesInCom();
	//��ȡ�������ݣ����س���
	//strData��������
	int ReadData(UINT bytesInQue, string& strData);
	//�ָ�����
	string& SplitSMS(string& strParam, const string &strSource);
	//PDU���Ž���
	void Decode(string& strParam);
	//�����ڶ��������ݼ�����������б���
	void AddList(const string strData);
	//��Ϣ����
	INT MessageHandler(SM_PARAM& smParam);
	//���ڼ����߳�
	static UINT WINAPI CommListenThread(LPVOID lpParam);
	int ReadProc(string& strData);
	//���ݴ����߳�
	static UINT WINAPI WriteDataThread(LPVOID lpParam);
	void WriteDatabase();
	INT Write(char *szParam);
	//����è״̬����߳�
	static UINT WINAPI MallWatchThread(LPVOID lpParam);
	int WatchProc();

public:
	//�رմ��ڼ����߳�
	void CloseListenThread();
	//�ر����ݶ�ȡ�߳�
	void CloseWriteThread();
	//�رն���è����߳�
	void CloseWatchThread();	
	
};


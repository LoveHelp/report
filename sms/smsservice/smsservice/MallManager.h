#pragma once

#include "Utils.h"
#include "SmsHelper.h"
//#include "MySQLHelper.h"
#include "HttpUtils.h"

#include <list>
using namespace std;

//串口信息结构体
struct COMMINFO
{
	INT nComm;
	INT nBaund;
	INT nParity;
	INT nDatabit;
	INT nStopbit;
};

//短信信息结构体
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
	/** 串口句柄 */
	HANDLE m_hComm;
	/** 串口监听线程 */
	HANDLE m_hListenThread;
	/** 消息处理线程句柄 */
	HANDLE m_hWriteDatabaseThread;
	/** 短信猫状态监控线程句柄 */
	HANDLE m_hMallWatchThread;
	/** 串口监听线程退出标志 */
	static bool m_bExitA;
	/** 数据处理线程退出标志 */
	static bool m_bExitB;
	/** 短信猫状态监控线程退出标志 */
	static bool m_bExitC;
	/** 串口打开标志 */
	bool m_bOpen;
	/** 同步互斥,临界区保护(串口) */
	CRITICAL_SECTION m_csCommunicationSync;
	/** 错误码 */
	int m_errno;
	/** 串口信息 */
	COMMINFO m_commInfo;
	/** 短信中心号码 */
	char m_addr[16];
	/** 命令 */
	string m_cmd;
	/** 短信解码类 */
	CSmsHelper *m_pSmsHelper;
	/** Http请求类*/
	CHttpUtils *m_pHttpUtils;
	/** 同步互斥，临界区保护*/
	CRITICAL_SECTION m_csDataSync;
	/** 短信数据列表 */
	list<SMSINFO> m_dataList;
	/** 猫初始化状态：-1默认值 0为初始化 1初始化成功 */
	int m_initState;

public:
	//获取串口初始化参数
	void InitParam(char iniFile[]);
	//检测串口是否开启
	BOOL IsOpened();
	//打开串口
	BOOL OpenPort();
	//初始化串口
	BOOL InitPort();
	//关闭串口
	void ClosePort();
	//获取错误码
	INT GetErrNo();
	//错误信息
	void ErrorInfo(TCHAR err[], INT nLen);
	//连接猫
	BOOL ConnectMall();
	//初始化猫
	int InitMall(string& strData);

private:
	//向串口发送指令
	//strData为指令返回值
	BOOL WriteComm(string& strData);
	//获取串口输入缓存中数据长度
	UINT GetBytesInCom();
	//读取串口数据，返回长度
	//strData串口数据
	int ReadData(UINT bytesInQue, string& strData);
	//分隔短信
	string& SplitSMS(string& strParam, const string &strSource);
	//PDU短信解析
	void Decode(string& strParam);
	//将串口读到的数据加入短信数据列表中
	void AddList(const string strData);
	//消息处理
	INT MessageHandler(SM_PARAM& smParam);
	//串口监听线程
	static UINT WINAPI CommListenThread(LPVOID lpParam);
	int ReadProc(string& strData);
	//数据处理线程
	static UINT WINAPI WriteDataThread(LPVOID lpParam);
	void WriteDatabase();
	INT Write(char *szParam);
	//短信猫状态监控线程
	static UINT WINAPI MallWatchThread(LPVOID lpParam);
	int WatchProc();

public:
	//关闭串口监听线程
	void CloseListenThread();
	//关闭数据读取线程
	void CloseWriteThread();
	//关闭短信猫监控线程
	void CloseWatchThread();	
	
};


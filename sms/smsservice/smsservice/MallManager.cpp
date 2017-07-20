#include "stdafx.h"
#include "MallManager.h"

#define SLEEPTIME_MIN 1000
//#define CURRENT_YEAR_2 "20"

bool CMallManager::m_bExitA = false;
bool CMallManager::m_bExitB = false;
bool CMallManager::m_bExitC = false;

CMallManager::CMallManager(void)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	m_hWriteDatabaseThread = INVALID_HANDLE_VALUE;
	m_hMallWatchThread = INVALID_HANDLE_VALUE;
	
	InitializeCriticalSection(&m_csCommunicationSync);
	InitializeCriticalSection(&m_csDataSync);

	memset(&m_commInfo, 0, sizeof(COMMINFO));
	memset(m_addr, 0, sizeof(m_addr));

	m_pSmsHelper = new CSmsHelper;
	m_pHttpUtils = new CHttpUtils;

	m_initState = -1;
	m_bOpen = false;
	m_errno = NO_ERROR;
}

CMallManager::~CMallManager() {
	//关闭线程
	CloseWatchThread();
	CloseListenThread();
	CloseWriteThread();
	//释放临界区
	DeleteCriticalSection(&m_csDataSync);
	DeleteCriticalSection(&m_csCommunicationSync);
	//释放堆空间
	if (NULL != m_pSmsHelper) 
	{
		delete m_pSmsHelper;
		m_pSmsHelper = NULL;
	}
	if (NULL != m_pHttpUtils)
	{
		delete m_pHttpUtils;
		m_pHttpUtils = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMallManager::InitParam(char iniFile[])
{
	char scAppName[] = {"COM"};
	char tszAddr[16] = "0";
	m_commInfo.nComm = 1;
	m_commInfo.nBaund = 115200;
	m_commInfo.nParity = 0;
	m_commInfo.nDatabit = 8;
	m_commInfo.nStopbit = 0;
	strcpy_s(m_addr, 16*sizeof(char), "8615893553324");

	m_commInfo.nComm = GetPrivateProfileIntA(scAppName, "com", 1, iniFile);
	m_commInfo.nBaund = GetPrivateProfileIntA(scAppName, "baund", 115200, iniFile);
	m_commInfo.nParity = GetPrivateProfileIntA(scAppName, "parity", 0, iniFile);
	m_commInfo.nDatabit = GetPrivateProfileIntA(scAppName, "databit", 8, iniFile);
	m_commInfo.nStopbit = GetPrivateProfileIntA(scAppName, "stopbit", 0, iniFile);
	GetPrivateProfileStringA(scAppName, "smc", "13800377500", m_addr, sizeof(m_addr), iniFile);

	//m_pMySQLHelper->InitParam(iniFile);
	m_pHttpUtils->InitParam(iniFile);
}

BOOL CMallManager::InitPort()
{
	//串口未开启
	if(!this->IsOpened())
	{
		this->m_errno = 0;
		return 1;
	}

	//进入临界区
	EnterCriticalSection(&m_csCommunicationSync);
	//设置缓冲区大小
	if (!SetupComm(m_hComm, 10240, 1024))
	{
		m_errno = GetLastError();
		LeaveCriticalSection(&m_csCommunicationSync);
		return 2;
	}

	//设置超时
	COMMTIMEOUTS timeout;
	GetCommTimeouts(m_hComm, &timeout);
	timeout.ReadIntervalTimeout = 100;//两字符之间最大的延时。0表示该参数不起作用。
	timeout.ReadTotalTimeoutConstant = 1;//一次读取串口数据的固定超时
	timeout.ReadTotalTimeoutMultiplier = 500;//读取每字符间的超时
	timeout.WriteTotalTimeoutConstant = 1;//一次写入串口数据的固定超时。
	timeout.WriteTotalTimeoutMultiplier = 100;//写入每字符间的超时。
	if (!SetCommTimeouts(m_hComm, &timeout))
	{
		m_errno = GetLastError();
		LeaveCriticalSection(&m_csCommunicationSync);
		return 3;
	}

	DCB dcb;
	memset(&dcb, 0, sizeof(dcb));
	if (!GetCommState(m_hComm, &dcb))
	{
		m_errno = GetLastError();
		LeaveCriticalSection(&m_csCommunicationSync);
		return 4;
	}

	dcb.BaudRate = m_commInfo.nBaund;
	dcb.Parity = m_commInfo.nParity;
	dcb.ByteSize = m_commInfo.nDatabit;
	dcb.StopBits = ONESTOPBIT;
	if (m_commInfo.nStopbit == 1)
		dcb.StopBits = ONE5STOPBITS;
	else if (m_commInfo.nStopbit == 2)
		dcb.StopBits = TWOSTOPBITS;
	if (!SetCommState(m_hComm, &dcb))
	{
		m_errno = GetLastError();
		LeaveCriticalSection(&m_csCommunicationSync);
		return 5;
	}

	//清空串口缓冲区
	PurgeComm(this->m_hComm, /*PURGE_RXCLEAR | PURGE_RXABORT |*/ PURGE_TXCLEAR | PURGE_TXABORT);
	//离开临界区
	LeaveCriticalSection(&m_csCommunicationSync);

	return 0;
}

BOOL CMallManager::IsOpened()
{
	return m_bOpen;
}

BOOL CMallManager::OpenPort()
{
	::EnterCriticalSection(&m_csCommunicationSync);
	TCHAR scComm[32] = _T("0");
	_stprintf_s(scComm, _T("COM%d"), m_commInfo.nComm);
	m_hComm = CreateFile(scComm,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		//                         FILE_FLAG_OVERLAPPED,
		0,
		NULL);

	m_bOpen = true;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		m_errno = GetLastError();
		m_bOpen = false;
	}
	::LeaveCriticalSection(&m_csCommunicationSync);
	return m_bOpen;
}

void CMallManager::ClosePort()
{
	::EnterCriticalSection(&m_csCommunicationSync);
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
	::LeaveCriticalSection(&m_csCommunicationSync);
}

INT CMallManager::GetErrNo()
{
	return this->m_errno;
}

void CMallManager::ErrorInfo(TCHAR err[], INT nLen)
{
	//TCHAR szBuf[128] = { 0 };
	LPVOID lpMsgBuf;
	DWORD dwError = m_errno;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPWSTR)&lpMsgBuf,
		0,
		NULL);
	//MessageBox(NULL, (LPWSTR)lpMsgBuf, TEXT("系统错误"), MB_OK | MB_ICONSTOP);
	memset(err, 0, nLen-1);
	memcpy(err, (LPCTSTR)lpMsgBuf, nLen-1);
	LocalFree(lpMsgBuf);
}

BOOL CMallManager::ConnectMall()
{
	BOOL bRet = FALSE;
	BOOL bInitMall = FALSE;
	string strData;
	::EnterCriticalSection(&m_csCommunicationSync);
	m_cmd = "AT\r\n";
	bRet = this->WriteComm(strData);
	if (!bRet || strData.find("OK") == string::npos)
	{
		Sleep(100);
		m_cmd = "AT\r\n";
		bRet = this->WriteComm(strData);
	}
	if (!bRet || strData.find("OK") == string::npos)
	{
		::LeaveCriticalSection(&m_csCommunicationSync);
		return FALSE;
	}

	//设置短信格式为PDU
	m_cmd = "AT+CMGF=0\r\n";
	bRet = this->WriteComm(strData);
	if (strData.find("OK") != -1)
		bRet = TRUE;

	if (bRet)
	{
		//关闭回显
		m_cmd = "ATE1\r\n";
		this->WriteComm(strData);
		//初始化猫配置
		bInitMall = InitMall(strData);
	}
	::LeaveCriticalSection(&m_csCommunicationSync);
	//连接猫成功之后，读取SIM卡中存储的所有短信
	//服务停止，猫一直在线，收到的短信会存储到串口输入缓冲区中
	//所以不必对SIM卡进行读取和删除操作
	//if (bRet)
	//{
	//	ReadSMS(strData);
	//	if (!strData.empty()
	//		&& strData.find("ERROR") == string::npos)
	//		DeleteSMS(0);//删除所有已读及已发送短信
	//}

	if (bRet)
	{
		m_bExitB = false;
		CUtils::Log(_T("[INFO]Write Database Thread is starting ..."));
		m_hWriteDatabaseThread = (HANDLE)_beginthreadex(NULL, 0, CMallManager::WriteDataThread, this, 0, NULL);
		m_bExitC = false;
		CUtils::Log(_T("[INFO]Mall Watch Thread is starting ..."));
		m_hMallWatchThread = (HANDLE)_beginthreadex(NULL, 0, CMallManager::MallWatchThread, this, 0, NULL);
		m_bExitA = false;
		CUtils::Log(_T("[INFO]Communication Listen Thread is starting ..."));
		m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, CMallManager::CommListenThread, this, 0, NULL);
	}
	return bRet;
}

int CMallManager::InitMall(string& strData)
{
	string strTmp;
	m_cmd = "AT+CSMS=0\r\n";
	this->WriteComm(strData);

	m_cmd = "AT+CNMI=2,2,0,1,1\r\n";
	this->WriteComm(strData);
	if (strData.find("AT+CNMI=2,2,0,1,1||OK") != string::npos)
		return 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL CMallManager::WriteComm(string& strData)
{
	BOOL   bResult = TRUE;
	DWORD  BytesToSend = 0;
	DWORD dwError = 0;

	int nLen = m_cmd.size();
	//Clear Com Error
	PurgeComm(m_hComm, /*PURGE_RXABORT | PURGE_RXCLEAR | */PURGE_TXABORT | PURGE_TXCLEAR);
	/*if (ClearCommError(m_hComm, &dwError, NULL) && dwError > 0)
	{
		PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
	}*/
	//Write Data
	bResult = WriteFile(m_hComm, (void *)m_cmd.data(), (DWORD)nLen, &BytesToSend, NULL);
	m_cmd = CUtils::Replace_All(m_cmd, "\r\n", "");
	//不显示AT+CFUN?的日志
	if(m_cmd.find("AT+CFUN?") == string::npos)
		CUtils::Log(_T("[INFO]Bytes Send: %d, Order: %S"), BytesToSend, m_cmd.data());
	if (!bResult)
	{
		this->m_errno = GetLastError();
		TCHAR tszErr[1024];
		this->ErrorInfo(tszErr, 1024);
		CUtils::Log(_T("[ERROR]SerialPort Writes Failed, Error Code: %d, Error Message: %s"), m_errno, tszErr);
		return FALSE;
	}

	int i = 0;
	UINT bytesInQue = 0;
	//发送指令后最多等待5s
	do 
	{
		Sleep(SLEEPTIME_MIN);
		i++;
		bytesInQue = GetBytesInCom();
		if (bytesInQue > 0)
			break;
	} while (i < 5);
	
	if (bytesInQue > 0) {
		ReadData(bytesInQue, strData);
	}

	return TRUE;
}

UINT CMallManager::GetBytesInCom()
{
	DWORD dwErr = 0;
	COMSTAT comstat;
	memset(&comstat, 0, sizeof(comstat));
	UINT bytesInQue = 0;
	//清除串口错误
	if (ClearCommError(m_hComm, &dwErr, &comstat))
	{
		bytesInQue = comstat.cbInQue;
	}
	return bytesInQue;
}

int CMallManager::ReadData(UINT bytesInQue, string& strData)
{
	CHAR buff[1024] = { 0 };
	DWORD dwByteRead = 0;
	DWORD dwByteAll = 0;
	DWORD dwBuffSize = 1024;
	DWORD dwlen = 0;

	BOOL bFlag = FALSE;
	if (m_cmd.find("CMGL") != string::npos)
		bFlag = TRUE;

	strData.clear();
	while (bytesInQue > 0)
	{
		memset(buff, 0, 1024);
		bool bRes = ReadFile(m_hComm, (LPVOID)buff, bytesInQue, &dwByteRead, NULL);
		dwByteAll = dwByteAll + dwByteRead;
		if (!bRes)
		{
			m_errno = GetLastError();
			//清空串口缓冲区
			PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
			break;
		}
		strData.append(buff);

		Sleep(SLEEPTIME_MIN);
		bytesInQue = GetBytesInCom();
	}
	if (!strData.empty())
	{
		strData = CUtils::Replace_All(strData, "\r\n", "|");
		//AT+CFUN?||+CFUN: 1||OK|长度为24
		if(strData.find("+CFUN: 1||OK") == string::npos 
			|| strData.size() > 23)
			CUtils::Log(_T("[INFO]Bytes Read: %d, Data: %S"), dwByteAll, strData.data());
		//解析读取到的数据，保存其中的短信PDU串
		AddList(strData);
	}
	return strData.size();
}

string& CMallManager::SplitSMS(string& strParam, const string &strSource)
{
	string strData = strSource;
	string strTmp;
	size_t ipos = strData.find("|");

	strParam.clear();
	if (strData.find("CMGL") == string::npos 
		&& strData.find("CMT") == string::npos
		&& strData.find("CMGR") == string::npos)
		return strParam;
	while (TRUE)
	{
		if (ipos == string::npos)
			break;
		strTmp = strData.substr(0, ipos);
		strData = strData.substr(ipos + 1);
		ipos = strData.find("|");
		if (strTmp.empty())
			continue;
		//去除命令回执信息
		else if (strTmp.find("AT") != string::npos)
			continue;
		else if (strTmp.find("CM") != string::npos)
			continue;
		else if (strTmp.find("OK") != string::npos)
			continue;
		else if(strTmp.find("CSMS") != string::npos)
			continue;
		else if (strTmp.find("CFUN") != string::npos)
			continue;
		else if (strTmp.find("RING") != string::npos)
			continue;

		strParam += strTmp + ";";
	}
	return strParam;
}

void CMallManager::Decode(string& strParam)
{
	int nRet = 0;
	string strTime, strTmp;
	TCHAR *tszTmp = NULL;
	int nLen = 0;
	
	SM_PARAM scm;
	memset(&scm, 0, sizeof(SM_PARAM));
	
	size_t ipos = strParam.find(";");
	while (ipos != string::npos)
	{
		strTmp = strParam.substr(0, ipos);
		strParam = strParam.substr(ipos + 1);
		ipos = strParam.find(";");
		//短信PDU串的长度>=30
		if(strTmp.length() < 30)
			continue;
		nLen = m_pSmsHelper->Decode(strTmp.data(), &scm);
		if(nLen == 0)
			continue;
		strTime = scm.TP_SCTS;
		strTime = strTime.substr(0, 12);
		strcpy_s(scm.TP_SCTS, 16, strTime.data());

		nLen = MultiByteToWideChar(CP_ACP, 0, scm.TP_UD, -1, NULL, 0);
		tszTmp = new TCHAR[nLen + 1];
		memset(tszTmp, 0, sizeof(TCHAR)*(nLen + 1));
		MultiByteToWideChar(CP_ACP, 0, scm.TP_UD, -1, tszTmp, nLen);

		CUtils::Log(_T("[SMS]From: %S, Time: %S, Info: %s"), scm.TPA, scm.TP_SCTS, tszTmp);
		delete[] tszTmp;
		tszTmp = NULL;

		nRet = this->MessageHandler(scm);
	}
}

void CMallManager::AddList(const string strData)
{
	string strDest, strSource=strData;
	if (strData.empty())
		return;
	//获取所有的PDU串
	strDest = SplitSMS(strDest, strSource);
	if (!strDest.empty())
	{
		Decode(strDest);
	}
}

INT CMallManager::MessageHandler(SM_PARAM& smParam)
{
	map<string, string> dataMap;
	string sep = " ";
	string strResult;
	CHAR szParam[1024] = { 0 };
	
	memset(szParam, 0, sizeof(szParam));
	sprintf_s(szParam, "reporter=%s&reporttype=2&bx=%s", smParam.TPA, smParam.TP_UD);

	//保存错误短信
	EnterCriticalSection(&m_csDataSync);
	SMSINFO smi;
	smi.nCount = 0;
	smi.strTxt = szParam;
	m_dataList.push_back(smi);
	m_dataList.unique();
	//CUtils::Log(_T("[MYSQL]Data Size: %d"), m_dataList.size());
	LeaveCriticalSection(&m_csDataSync);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
UINT CMallManager::CommListenThread(LPVOID lpParam)
{
	string strParam, strData;
	CMallManager *pMall = (CMallManager *)lpParam;
	if (NULL == pMall)
	{
		CUtils::Log(_T("[ERROR]Communication Listen Thread starts Failed: Object is Null"));
		return -1;
	}

	while (!m_bExitA)
	{
		strParam.clear();
		pMall->ReadProc(strData);
		Sleep(SLEEPTIME_MIN * 3);
	}
	return 0;
}

int CMallManager::ReadProc(string& strData)
{
	UINT nBytesInQue = 0;
	int nSize = 0;
	::EnterCriticalSection(&m_csCommunicationSync);
	nBytesInQue = GetBytesInCom();
	nSize = ReadData(nBytesInQue, strData);
	::LeaveCriticalSection(&m_csCommunicationSync);
	return nSize;
}

//////////////////////////////////////////////////////////////////////////
UINT CMallManager::WriteDataThread(LPVOID lpParam)
{
	CMallManager *pMall = (CMallManager*)lpParam;
	if (NULL == pMall)
	{
		CUtils::Log(_T("[ERROR]Write Database Thread starts Failed: Object is Null"));
		return -1;
	}

	while (!m_bExitB)
	{
		pMall->WriteDatabase();
		Sleep(SLEEPTIME_MIN + 237);
	}

	return 0;
}

void CMallManager::WriteDatabase()
{
	list<SMSINFO>::iterator ibegin, ipos, iend;
	INT bRes = 0;
	char szParam[1024] = { 0 };

	::EnterCriticalSection(&m_csDataSync);
	ibegin = m_dataList.begin();
	iend = m_dataList.end();
	while (ibegin != iend)
	{
		ipos = ibegin;
		ibegin++;
		strcpy_s(szParam, ipos->strTxt.data());
		if(ipos->nCount++<30)
			bRes = Write(szParam);
		else {
			CUtils::Log(_T("[MYSQL]Data '%S' Insert to DB failed for more than 30 times, Delete"), szParam);
			bRes = 0;
		}
			
		if (bRes == 0)
			m_dataList.erase(ipos);
	}
	::LeaveCriticalSection(&m_csDataSync);
}

INT CMallManager::Write(char *szParam)
{
	string strResult;
	bool bRes = m_pHttpUtils->PostData(szParam);
	if (bRes)
	{
		strResult.clear();
		strResult = m_pHttpUtils->GetResult();
		CUtils::Log(_T("[MYSQL]Parameters: %S"), szParam);
		strResult = CUtils::Replace_All(strResult, "\r\n", "|");
		CUtils::Log(_T("[MYSQL]Result: %S\n\n"), strResult.data());
		size_t npos = strResult.find("||");
		if (npos != string::npos)
		{
			if (strResult.length() > (npos + 2))
			{
				strResult = strResult.substr(npos+2);
				if (strResult.length() > 1)
					return 1;
				strResult = strResult.substr(0, 1);
				if (!strResult.empty())
				{
					if (atoi(strResult.data()) == 3)
						return 1;
				}
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
UINT CMallManager::MallWatchThread(LPVOID lpParam)
{
	CMallManager *pMall = (CMallManager*)lpParam;
	if (NULL == pMall)
	{
		CUtils::Log(_T("[ERROR]Mall Watch Thread starts Failed: Object is Null"));
		return -1;
	}
	
	while (!m_bExitC)
	{
		pMall->WatchProc();
		Sleep(SLEEPTIME_MIN * 4 + 111);
	}

	return 0;
}

int CMallManager::WatchProc()
{
	string strData, strMessage;
	::EnterCriticalSection(&m_csCommunicationSync);
	m_cmd = "AT+CFUN?\r\n";
	WriteComm(strData);
	if (strData.find("CFUN: 0") != string::npos)
	{
		m_cmd = "AT+CFUN=1\r\n";
		WriteComm(strData);
		if (strData.find("AT+CFUN=1") != string::npos
			&& strData.find("OK") != string::npos)
			m_initState = 0;
	}
	if (m_initState == 0)
		m_initState = InitMall(strData);
	else if (m_initState == 1)
	{
		m_initState = -1;
		m_cmd = "AT+CMGL=4\r\n";
		WriteComm(strData);
		//读取成功之后，删除SIM卡中所有短信
		if (strData.find("ERROR") == string::npos
			&& strData.length() > 17)
		{
			//删除已读及已发送短信
			m_cmd = "AT+CMGD=1,2\r\n";
			WriteComm(strData);
		}	
	}
	::LeaveCriticalSection(&m_csCommunicationSync);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMallManager::CloseListenThread()
{
	m_bExitA = true;
	if (INVALID_HANDLE_VALUE != m_hListenThread)
	{
		WaitForSingleObject(m_hListenThread, 3000);
		CloseHandle(m_hListenThread);
		m_hListenThread = INVALID_HANDLE_VALUE;
		CUtils::Log(_T("[INFO]Communication Listen Thread is Closed"));
	}
}

void CMallManager::CloseWriteThread()
{
	m_bExitB = true;
	if (INVALID_HANDLE_VALUE != m_hWriteDatabaseThread)
	{
		WaitForSingleObject(m_hWriteDatabaseThread, 3000);
		//设置线程句柄无效
		CloseHandle(m_hWriteDatabaseThread);
		m_hWriteDatabaseThread = INVALID_HANDLE_VALUE;
		CUtils::Log(_T("[INFO]Write Data Thread is Closed"));
	}
}

void CMallManager::CloseWatchThread()
{
	m_bExitC = true;
	if (INVALID_HANDLE_VALUE != m_hMallWatchThread)
	{
		WaitForSingleObject(m_hMallWatchThread, 3000);
		//设置线程句柄无效
		CloseHandle(m_hMallWatchThread);
		m_hMallWatchThread = INVALID_HANDLE_VALUE;
		CUtils::Log(_T("[INFO]Mall Watch Thread is Closed"));
	}
}
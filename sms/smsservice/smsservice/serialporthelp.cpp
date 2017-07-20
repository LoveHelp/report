#include "serialporthelp.h"

#include <process.h>
#include <QDebug>

bool SerialPortHelp::m_sbExit = false;

SerialPortHelp::SerialPortHelp()
{
    m_hComm = INVALID_HANDLE_VALUE;
    m_bOpen = false;
    m_errno = NO_ERROR;
    m_hListenThread = INVALID_HANDLE_VALUE;
    InitializeCriticalSection(&m_csCommunicationSync);
}

SerialPortHelp::~SerialPortHelp()
{
    closePort();
    DeleteCriticalSection(&m_csCommunicationSync);
}

bool SerialPortHelp::isOpened()
{
    return m_bOpen;
}

bool SerialPortHelp::openPort(char *portname)
{
    EnterCriticalSection(&m_csCommunicationSync);
    DWORD dwSize = MultiByteToWideChar(CP_ACP, 0, portname, -1, NULL, 0);
    DWORD dwlen = dwSize + 1;
    WCHAR *wszPort = new WCHAR[dwlen];
    memset(wszPort, 0, dwlen);
    MultiByteToWideChar(CP_ACP, 0, portname, -1, wszPort, dwlen);
    m_hComm = CreateFile(wszPort,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
//                         FILE_FLAG_OVERLAPPED,
                         0,
                         NULL);
    delete []wszPort;
    wszPort = NULL;

    m_bOpen = true;
    if(m_hComm == INVALID_HANDLE_VALUE)
    {
        m_errno = GetLastError();
        m_bOpen = false;
    }
    ::LeaveCriticalSection(&m_csCommunicationSync);
    return m_bOpen;
}

void SerialPortHelp::closePort()
{
    CloseListenThread();
    if(m_hComm != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hComm);
        m_hComm = INVALID_HANDLE_VALUE;
    }
}

int SerialPortHelp::initPort(int baud/*=115200*/, int parity/*=0*/, int databit/*=8*/, int stopbit/*=1*/, DWORD dwCommEvents/*=EV_RXCHAR*/)
{
    Q_UNUSED(dwCommEvents);
    if(!this->isOpened())
    {
        m_errno = 0;
        return 1;
    }

    //进入临界区
    EnterCriticalSection(&m_csCommunicationSync);
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    if(!GetCommState(m_hComm, &dcb))
    {
        m_errno = GetLastError();
        return 2;
    }

    dcb.BaudRate = baud;
    dcb.Parity = parity;
    dcb.ByteSize = databit;
    dcb.StopBits = ONESTOPBIT;
    if(stopbit == 1)
        dcb.StopBits = ONE5STOPBITS;
    else if(stopbit == 2)
        dcb.StopBits = TWOSTOPBITS;
    if(!SetCommState(m_hComm, &dcb))
    {
        m_errno = GetLastError();
        return 3;
    }

    COMMTIMEOUTS timeout;
    GetCommTimeouts(m_hComm, &timeout);
    timeout.ReadIntervalTimeout=0;//两字符之间最大的延时。0表示该参数不起作用。
    timeout.ReadTotalTimeoutConstant=0;//一次读取串口数据的固定超时
    timeout.ReadTotalTimeoutMultiplier=0;//读取每字符间的超时
    timeout.WriteTotalTimeoutConstant=0;//一次写入串口数据的固定超时。
    timeout.WriteTotalTimeoutMultiplier=0;//写入每字符间的超时。
    if(!SetCommTimeouts(m_hComm, &timeout))
    {
        m_errno = GetLastError();
        return 4;
    }

    if(!SetupComm(m_hComm, 1024, 1024))
    {
        m_errno = GetLastError();
        return 5;
    }
    //清空串口缓冲区
    PurgeComm(this->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    //离开临界区
    LeaveCriticalSection(&m_csCommunicationSync);

    return 0;
}

bool SerialPortHelp::OpenListenThread()
{
    if(m_hListenThread != INVALID_HANDLE_VALUE)
        return true;

    m_sbExit = false;
    UINT threadId = 0;

    m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
    if(!m_hListenThread)
    {
        m_errno = GetLastError();
        return false;
    }
    SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL);
    return true;
}

void SerialPortHelp::CloseListenThread()
{
    if(m_hListenThread != INVALID_HANDLE_VALUE)
    {
        m_sbExit = true;
        WaitForSingleObject(m_hListenThread, 3000);
        CloseHandle(m_hListenThread);
        m_hListenThread = INVALID_HANDLE_VALUE;
    }
}

UINT WINAPI SerialPortHelp::ListenThread(LPVOID pParam)
{
    SerialPortHelp *spHelp = reinterpret_cast<SerialPortHelp *>(pParam);
    while(!m_sbExit)
    {
        UINT bytesInQue = spHelp->GetBytesInCom();
        if(bytesInQue == 0)
        {
            Sleep(SLEEP_TIME_INTERVAL);
            continue;
        }
        spHelp->ReadData(bytesInQue);
    }
    return 0;
}

UINT SerialPortHelp::GetBytesInCom()
{
    DWORD dwErr=0;
    COMSTAT comstat;
    memset(&comstat, 0, sizeof(comstat));
    UINT bytesInQue = 0;
    //进入临界区
    EnterCriticalSection(&m_csCommunicationSync);
    if(ClearCommError(m_hComm, &dwErr, &comstat))
    {
        bytesInQue = comstat.cbInQue;
    }
    //离开临界区
    LeaveCriticalSection(&m_csCommunicationSync);
    return bytesInQue;
}

void SerialPortHelp::ReadData(UINT bytesInQue)
{
//    WCHAR szBuff[1024]={0};
    CHAR buff[1024]={0};
    DWORD dwByteRead = 0;
    DWORD dwBuffSize = 1023;
//    int nLen = 0;
    QString data="";
    //进入临界区
    EnterCriticalSection(&m_csCommunicationSync);

    if(dwBuffSize > bytesInQue)
        dwBuffSize = bytesInQue;
    while(dwBuffSize > 0)
    {
        memset(buff, 0, 1024);
        bool bRes = ReadFile(m_hComm, buff, dwBuffSize, &dwByteRead, NULL);
        if(!bRes)
        {
            //spHelp->SetErrNo(GetLastError());
            this->m_errno = GetLastError();
            emit errorRead();
            //清空串口缓冲区
            PurgeComm(m_hComm, PURGE_RXCLEAR|PURGE_RXABORT);
            break;
        }
        else{
//            memset(buff, 0, 1024);
//            nLen = wcslen(szBuff) * 2 + 1;
//            WideCharToMultiByte(CP_ACP, 0, szBuff, -1, buff, 1024, NULL, NULL);
//            data.append(buff);
            data.append(QString::fromLocal8Bit(buff));
            dwBuffSize = bytesInQue - dwBuffSize;
        }

    }
    if(!data.isEmpty()){
        data.append("\n");
        //data = data.toLocal8Bit();
        emit readyRead(data);
    }

    //离开临界区
    LeaveCriticalSection(&m_csCommunicationSync);

}

void SerialPortHelp::SetErrNo(int err)
{
    this->m_errno = err;
}

void SerialPortHelp::errinfo(char msg[])
{
    WCHAR szBuf[128] = { 0 };
    LPVOID lpMsgBuf;
    DWORD dwError = m_errno;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&lpMsgBuf,
                0,
                NULL);
    //MessageBox(NULL, (LPWSTR)lpMsgBuf, TEXT("系统错误"), MB_OK | MB_ICONSTOP);
    memcpy(szBuf, (LPCWSTR)lpMsgBuf, 127);
    INT len = wcslen(szBuf) * 2 + 1;
    WideCharToMultiByte(CP_UTF8, 0, szBuf, -1, msg, len, NULL, NULL);
    LocalFree(lpMsgBuf);
}

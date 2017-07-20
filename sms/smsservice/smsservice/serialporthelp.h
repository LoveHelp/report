#ifndef SERIALPORTHELP_H
#define SERIALPORTHELP_H

#include <QObject>
#include <windows.h>

/** 当串口无数据时,sleep至下次查询间隔的时间,单位:毫秒 */
#define SLEEP_TIME_INTERVAL 500
/** 读数据时间间隔 */
#define READ_TIME_INTERVAL 20

class SerialPortHelp : public QObject
{
    Q_OBJECT

public:
    SerialPortHelp();
    ~SerialPortHelp();

signals:
    void errorRead();
    void readyRead(QString data);

private:
    /** 串口句柄 */
    HANDLE m_hComm;

    /** 串口打开标志 */
    bool m_bOpen;

    /** 线程退出标志变量 */
    static bool m_sbExit;

    /** 线程句柄 */
    volatile HANDLE m_hListenThread;

    /** 同步互斥,临界区保护 */
    CRITICAL_SECTION m_csCommunicationSync;

    /** 错误码 */
    int m_errno;

public:
    //检测串口是否开启
    bool isOpened();

    //打开串口
    bool openPort(char *portName);

    //关闭串口
    void closePort();

    //开启监听线程
    bool OpenListenThread();

    //关闭监听线程
    void CloseListenThread();

    //串口监听线程
    static UINT WINAPI ListenThread(LPVOID pParam);

    //查询串口是否接收到数据
    UINT GetBytesInCom();

    //从串口读取数据
    void ReadData(UINT bytesInQue);

    //设置错误码
    void SetErrNo(int err);

    //初始化串口
    int initPort(int baud=115200, int parity=0, int databit=8, int stopbit=1, DWORD dwCommEvents=EV_RXCHAR);

    //获取系统错误原因
    void errinfo(char msg[]);

};

#endif // SERIALPORTHELP_H

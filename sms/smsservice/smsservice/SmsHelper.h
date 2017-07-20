#pragma once

// 用户信息编码方式
#define GSM_7BIT        0
#define GSM_8BIT        4
#define GSM_UCS2        8
// 短消息参数结构，编码/解码共用
// 其中，字符串以0结尾
typedef struct {
	char SCA[16];       // 短消息服务中心号码(SMSC地址)
	char TPA[16];       // 目标号码或回复号码(TP-DA或TP-RA)
	char TP_PID;        // 用户信息协议标识(TP-PID)
	char TP_DCS;        // 用户信息编码方式(TP-DCS)
	char TP_SCTS[16];   // 服务时间戳字符串(TP_SCTS), 接收时用到
	char TP_UD[161];    // 原始用户信息(编码前或解码后的TP-UD)
	char index;         // 短消息序号，在读取时用到
} SM_PARAM;
typedef struct {
	int len;
	char data[16384];
}SM_BUFF;
class CSmsHelper
{
public:
	CSmsHelper(VOID) {};
	~CSmsHelper() {};

private:
	// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
	int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength);
	// 两两颠倒的字符串转换为正常顺序的字符串
	int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength);

	// UCS2编码
	int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// UCS2解码
	int gsmDecodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength);
	
	// 字节数据转换为可打印字符串
	int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);
	// 可打印字符串转换为字节数据
	int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);

	// 7-bit编码
	int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// 7-bit解码
	int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

	// 8bit编码 
	int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// 8bit解码 
	int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

	// PDU编码，用于编制、发送短消息
	int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst);
	// PDU解码，用于接收、阅读短消息
	int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst);

public:
	// 短信PDU编码
	int Encode(const SM_PARAM *pSrc, char pdu[], int nLen);
	// 短信PDU解码
	int Decode(const char* pSrc, SM_PARAM* pDst);
	// 字节数据转换为可打印字符串
	int BytesToChar(const char *pSrc, char* pDst, int nLen);
};


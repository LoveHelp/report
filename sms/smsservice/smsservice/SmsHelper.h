#pragma once

// �û���Ϣ���뷽ʽ
#define GSM_7BIT        0
#define GSM_8BIT        4
#define GSM_UCS2        8
// ����Ϣ�����ṹ������/���빲��
// ���У��ַ�����0��β
typedef struct {
	char SCA[16];       // ����Ϣ�������ĺ���(SMSC��ַ)
	char TPA[16];       // Ŀ������ظ�����(TP-DA��TP-RA)
	char TP_PID;        // �û���ϢЭ���ʶ(TP-PID)
	char TP_DCS;        // �û���Ϣ���뷽ʽ(TP-DCS)
	char TP_SCTS[16];   // ����ʱ����ַ���(TP_SCTS), ����ʱ�õ�
	char TP_UD[161];    // ԭʼ�û���Ϣ(����ǰ�������TP-UD)
	char index;         // ����Ϣ��ţ��ڶ�ȡʱ�õ�
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
	// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
	int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength);
	// �����ߵ����ַ���ת��Ϊ����˳����ַ���
	int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength);

	// UCS2����
	int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// UCS2����
	int gsmDecodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength);
	
	// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
	int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);
	// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
	int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);

	// 7-bit����
	int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// 7-bit����
	int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

	// 8bit���� 
	int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
	// 8bit���� 
	int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

	// PDU���룬���ڱ��ơ����Ͷ���Ϣ
	int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst);
	// PDU���룬���ڽ��ա��Ķ�����Ϣ
	int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst);

public:
	// ����PDU����
	int Encode(const SM_PARAM *pSrc, char pdu[], int nLen);
	// ����PDU����
	int Decode(const char* pSrc, SM_PARAM* pDst);
	// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
	int BytesToChar(const char *pSrc, char* pDst, int nLen);
};


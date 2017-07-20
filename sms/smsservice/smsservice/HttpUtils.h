#pragma once
class CHttpUtils
{
public:
	CHttpUtils();
	~CHttpUtils() {};

private:
	char m_host[128];
	int m_port;
	char m_url[128];
	char m_path[32];
	char m_szResult[1024 * 1024];

private:
	bool SocketHttp(char *param);
	
public:
	void InitParam(char iniFile[]);
	bool PostData(char *data);
	bool GetData(char *data);
	char* GetResult();
};


#include "stdafx.h"
#include "HttpUtils.h"

#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <exception>

CHttpUtils::CHttpUtils()
{
	memset(m_host, 0, sizeof(m_host));
	sprintf_s(m_host, "127.0.0.1");
	m_port = 8081;
	memset(m_url, 0, sizeof(m_url));
	sprintf_s(m_url, "localhost:8081");
	memset(m_path, 0, sizeof(m_path));
	sprintf_s(m_path, "/checkbx.php");

	memset(m_szResult, 0, sizeof(m_szResult));

	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

bool CHttpUtils::SocketHttp(char* param)
{
	int sockfd;
	struct sockaddr_in address;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_port = htons(m_port);
	inet_pton(AF_INET, m_host, &address.sin_addr);
	//address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (-1 == connect(sockfd, (struct sockaddr *)&address, sizeof(address)))
		return false;

	send(sockfd, param, strlen(param), 0);

	memset(m_szResult, 0, sizeof(m_szResult));

	int offset = 0;
	int rc;

	while (rc = recv(sockfd, m_szResult + offset, 1024, 0))
		offset += rc;

	closesocket(sockfd);

	m_szResult[offset] = '\0';
	//std::cout << "Recv Data: " << buff << std::endl;

	return true;
}

void CHttpUtils::InitParam(char iniFile[])
{
	char scAppName[] = { "SERVER" };
	memset(m_host, 0, sizeof(m_host));
	sprintf_s(m_host, "127.0.0.1");
	m_port = 80;

	GetPrivateProfileStringA(scAppName, "addr", "127.0.0.1", m_host, sizeof(m_host), iniFile);
	m_port = GetPrivateProfileIntA(scAppName, "port", 8081, iniFile);
	GetPrivateProfileStringA(scAppName, "url", "localhost:8081", m_url, sizeof(m_url), iniFile);
	GetPrivateProfileStringA(scAppName, "path", "/checkbx.php", m_path, sizeof(m_path), iniFile);
}

bool CHttpUtils::PostData(char * data)
{
	char szHost[64] = { 0 };
	char szData[1024] = { 0 };
	char szTmp[16] = { 0 };
	int nLen = 0;
	bool bRes = false;
	try
	{
		nLen = strlen(data);

		memset(m_szResult, 0, sizeof(m_szResult));

		memset(szHost, 0, sizeof(szHost));
		sprintf_s(szHost, "%s", m_url);

		memset(szData, 0, sizeof(szData));
		strcat_s(szData, "POST ");
		strcat_s(szData, m_path);
		strcat_s(szData, " HTTP/1.0\r\n");

		strcat_s(szData, "Host: ");
		strcat_s(szData, m_url);
		strcat_s(szData, "\r\n");

		strcat_s(szData, "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:7.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n");
		strcat_s(szData, "Content-type:application/x-www-form-urlencoded\r\n");
		//std::cout << szData << std::endl;

		strcat_s(szData, "Content-Length: ");
		sprintf_s(szTmp, "%d", nLen);
		strcat_s(szData, szTmp);
		strcat_s(szData, "\r\n");
		//std::cout << szData << std::endl;

		strcat_s(szData, "Connection:close\r\n\r\n");
		strcat_s(szData, data);
		//std::cout << szData << std::endl;

		bRes = this->SocketHttp(szData);
	}
	catch (std::exception& e)
	{
		bRes = false;
	}
	
	return bRes;
}

bool CHttpUtils::GetData(char * data)
{
	char szHost[64] = { 0 };
	char szData[1024] = { 0 };
	bool bRes = false;
	try
	{
		memset(m_szResult, 0, sizeof(m_szResult));

		memset(szHost, 0, sizeof(szHost));
		sprintf_s(szHost, "%s", m_url);

		memset(szData, 0, sizeof(szData));
		strcat_s(szData, "GET ");
		strcat_s(szData, m_path);
		strcat_s(szData, "?");
		strcat_s(szData, data);
		strcat_s(szData, " HTTP/1.0\r\n");

		strcat_s(szData, "Host: ");
		strcat_s(szData, m_url);
		strcat_s(szData, "\r\n");

		strcat_s(szData, "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:7.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n");

		strcat_s(szData, "Connection:close\r\n\r\n");

		bRes = SocketHttp(szData);
	}
	catch (std::exception& e)
	{
		bRes = false;
	}
	
	return bRes;
}

char * CHttpUtils::GetResult()
{
	return m_szResult;
}

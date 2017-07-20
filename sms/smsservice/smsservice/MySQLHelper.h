#pragma once
#include "mysql.h"
#pragma comment(lib, "libmysql.lib")

struct MYSQLERROR
{
	int no;
	char txt[1024];
};

class CMySQLHelper
{
public:
	CMySQLHelper() { m_conn = NULL; };
	CMySQLHelper(const char *addr, const char *user, const char *pwd, const char *db, const int port)
	{
		strcpy_ms(m_addr, addr, sizeof(m_addr));
		
		strcpy_ms(m_user, user, sizeof(m_user));

		strcpy_ms(m_passwd, pwd, sizeof(m_passwd));

		strcpy_ms(m_db, db, sizeof(m_db));

		m_port = 3306;
		if (0 != port)
		{
			m_port = port;
		}
	};
	~CMySQLHelper() { Disconnect(); };

private:
	MYSQL* m_conn;

	char m_addr[16];
	char m_user[16];
	char m_passwd[32];
	char m_db[32];
	int m_port;

	MYSQLERROR m_err;

private:
	int strcpy_ms(char *dest, const char *src, int len);

public:
	void InitParam(char iniFile[]);
	BOOL Connect();
	BOOL IsConnect();
	void Disconnect();
	BOOL BeginTransaction();
	BOOL Commit();
	BOOL Rollback();
	INT Insert(const char *sql);
	INT Delete(const char *sql);

	BOOL IfExists(const char *sql);

	MYSQLERROR& GetError();
};


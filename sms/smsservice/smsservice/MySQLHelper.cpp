#include "stdafx.h"
#include "MySQLHelper.h"
#include "Utils.h"

#include <io.h>

int CMySQLHelper::strcpy_ms(char * dest, const char * src, int len)
{
	int nsize = strlen(src);
	if (nsize >= len)
		nsize = len - 1;

	memset(dest, 0, len * sizeof(char));
	for (int i = 0; i < nsize; i++)
	{
		dest[i] = src[i];
	}
	dest[nsize] = '\0';
	
	return 0;
}

void CMySQLHelper::InitParam(char iniFile[])
{
	char czAppName[] = "MYSQL";
	GetPrivateProfileStringA(czAppName, "addr", "localhost", m_addr, sizeof(m_addr), iniFile);
	GetPrivateProfileStringA(czAppName, "user", "root", m_user, sizeof(m_user), iniFile);
	GetPrivateProfileStringA(czAppName, "password", "s123", m_passwd, sizeof(m_passwd), iniFile);
	GetPrivateProfileStringA(czAppName, "database", "waterbureau", m_db, sizeof(m_db), iniFile);
	m_port = GetPrivateProfileIntA(czAppName, "port", 3306, iniFile);
	
}

BOOL CMySQLHelper::Connect()
{
	m_conn = mysql_init(NULL);
	int flag = 1;
	//mysql_options(m_conn, MYSQL_OPT_READ_TIMEOUT, (const char*)&flag);
	mysql_options(m_conn, MYSQL_OPT_RECONNECT, (const char*)&flag);
	CUtils::Log(_T("[MYSQL]addr:%S, user: %S, passwd: %S, db: %S, port: %d"), m_addr, m_user, m_passwd, m_db, m_port);
	if (!mysql_real_connect(m_conn, m_addr, m_user, m_passwd, m_db, m_port, NULL, 0))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return FALSE;
	}
	return TRUE;
}

BOOL CMySQLHelper::IsConnect()
{
	if (NULL == m_conn) {
		return Connect();
	}
	CUtils::Log(_T("[MYSQL]thread id: %d"), mysql_thread_id(m_conn));
	int nRes = mysql_ping(m_conn);
	CUtils::Log(_T("[MYSQL]ping: %d, thread id: %d"), nRes, mysql_thread_id(m_conn));
	return TRUE;
}

void CMySQLHelper::Disconnect()
{
	if (m_conn != NULL)
	{
		mysql_close(m_conn);
		m_conn = NULL;
	}
}

BOOL CMySQLHelper::BeginTransaction()
{
	char sql[] = { "begin" };
	if (mysql_query(m_conn, sql))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return FALSE;
	}
	return TRUE;
}

BOOL CMySQLHelper::Commit()
{
	if (mysql_commit(m_conn))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return FALSE;
	}
	return TRUE;
}

BOOL CMySQLHelper::Rollback()
{
	if (mysql_rollback(m_conn))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return FALSE;
	}
	return TRUE;
}

INT CMySQLHelper::Insert(const char *sql)
{
	int nId = 0;
	if (mysql_query(m_conn, sql))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return -1;
	}
	nId = (INT)mysql_insert_id(m_conn);
	return nId;
}

INT CMySQLHelper::Delete(const char *sql)
{
	INT rowCount = 0;
	if (mysql_query(m_conn, sql))
	{
		m_err.no = mysql_errno(m_conn);
		strcpy_ms(m_err.txt, mysql_error(m_conn), sizeof(m_err.txt));
		return -1;
	}
	rowCount = (INT)mysql_affected_rows(m_conn);
	return rowCount;
}

BOOL CMySQLHelper::IfExists(const char * sql)
{
	int nId = 0;
	int nSize = 0;
	if (mysql_query(m_conn, sql) == 0)
	{
		MYSQL_RES *res = mysql_store_result(m_conn);
		if (mysql_num_rows(res) > 0) 
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			nSize = atoi(row[0]);
		}
		mysql_free_result(res);
		CUtils::Log(_T("[MYSQL]query result: %d"), nSize);
		if(nSize == 1)
			return TRUE;
	}
	return FALSE;
}

MYSQLERROR & CMySQLHelper::GetError()
{
	// TODO: 在此处插入 return 语句
	return this->m_err;
}

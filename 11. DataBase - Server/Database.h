#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "server.h"



class GameServer;
class Database : public Singleton<Database>
{
public:
	Database();
	~Database();

	void DatabaseThread();

	bool Login(UINT uid, const char* id, const char* password);
	bool Logout(UINT uid);
	bool UpdateUserData(UINT uid, INT x, INT y);

private:
	void ShowError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retcode);

private:
	const wstring	DatabaseName = TEXT("GameServerExample");
	concurrency::concurrent_unordered_map<int, string> m_id;

	// DB 핸들에 직접 접근 금지 (Locking 필수)
	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;
	mutex		handleLock;
	SQLRETURN	retcode;
	SQLWCHAR	szName[NAMESIZE];
};


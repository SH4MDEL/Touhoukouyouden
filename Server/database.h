#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "server.h"

struct DataBaseUserInfo
{
	char* id;
	char* password;
	int xPosition;
	int yPosition;
	unsigned char serial;
};

struct DatabaseEvent
{
	enum Type { LOGIN, LOGOUT, UPDATE, SIGNUP };

	UINT m_id;
	Type m_type;
	DataBaseUserInfo m_dbInfo;
};

class GameServer;
class Database : public Singleton<Database>
{
public:
	Database();
	~Database();

	void DatabaseThread(HANDLE hiocp);
	void AddDatabaseEvent(const DatabaseEvent& userInfo);

private:
	// DB 직접 접근은 DB Thread에서만.
	bool Login(UINT uid, const char* id, const char* password);
	bool Logout(UINT uid, INT x, INT y);
	bool UpdateUserData(UINT uid, INT x, INT y);
	bool Signup(const char* id, const char* password, const int serial);

	void ShowError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retcode);

private:
	const wstring	DatabaseName = TEXT("GameServerExample");
	concurrency::concurrent_unordered_map<int, string> m_id;

	concurrency::concurrent_queue<DatabaseEvent> m_dbQueue;
	
	// DB 핸들에 직접 접근 금지 (Locking 필수)
	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;
	mutex		handleLock;
	SQLRETURN	retcode;
	SQLWCHAR	szName[NAME_SIZE];
};


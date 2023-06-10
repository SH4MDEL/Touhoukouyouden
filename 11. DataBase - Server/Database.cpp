#include "Database.h"

/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/* hHandle ODBC handle
/* hType Type of handle (SQL_HANDLE_STMT, SQL_HANDLE_ENV, SQL_HANDLE_DBC)
/* RetCode Return code of failing command
/************************************************************************/

Database::Database()
{
    setlocale(LC_ALL, "korean");

    cout << "Initialize Database begin\n";

    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLWCHAR*)DatabaseName.c_str(), SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    cout << "Initialize Database end\n";
                }
            }
        }
    }
}

Database::~Database()
{
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void Database::DatabaseThread()
{

}

bool Database::Login(UINT uid, const char* id, const char* password)
{
    // 해당 ID가 이미 접속중인 경우 로그인 실패
    if (m_id.count(uid)) return false;

    handleLock.lock();
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    SQLWCHAR wstr[100];
    memset(wstr, 0, sizeof(wstr));
    wsprintf(wstr, TEXT("EXEC Login %S, %S"), id, password);
    retcode = SQLExecDirect(hstmt, wstr, SQL_NTS);

    SQLINTEGER xPosition, yPosition;
    SQLLEN Xlen = 0, Ylen = 0;

    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &xPosition, 10, &Xlen);
        retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &yPosition, 10, &Ylen);

        retcode = SQLFetch(hstmt); // 데이터를 하나씩 꺼냄
        if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
            ShowError(hstmt, SQL_HANDLE_STMT, retcode);
            handleLock.unlock();
            return false;
        }
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            // ID가 이미 접속중일경우 예외처리 해줘야함.
            // 어떻게?
            g_gameServer.RegistClientPosition(uid, { (short)xPosition, (short)yPosition });
            handleLock.unlock();
            m_id.insert({ uid, id });
            return true;
        }
        else {
            handleLock.unlock();
            return false;
        }
    }
    else {
        ShowError(hstmt, SQL_HANDLE_STMT, retcode);
        handleLock.unlock();
        return false;
    }
}

bool Database::Logout(UINT uid)
{
    // 이미 id가 존재하지 않으면 로그아웃 실패
    if (!m_id.count(uid)) return false;

    handleLock.lock();
    m_id.unsafe_erase(uid);
    handleLock.unlock();
    return true;
}

bool Database::UpdateUserData(UINT uid, INT x, INT y)
{
    // id가 존재하지 않으면 저장 실패
    if (!m_id.count(uid)) return false;
    
    handleLock.lock();
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    SQLWCHAR wstr[100];
    memset(wstr, 0, sizeof(wstr));
    wsprintf(wstr, TEXT("EXEC UpdateUserData %S, %d, %d"), m_id[uid].c_str(), x, y);
    retcode = SQLExecDirect(hstmt, wstr, SQL_NTS);


    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
            ShowError(hstmt, SQL_HANDLE_STMT, retcode);
            handleLock.unlock();
            return false;
        }
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            handleLock.unlock();
            return true;
        }
        else {
            handleLock.unlock();
            return false;
        }
    }
    else {
        ShowError(hstmt, SQL_HANDLE_STMT, retcode);
        handleLock.unlock();
        return false;
    }

}

void Database::ShowError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retcode)
{
    SQLSMALLINT rec = 0;
    SQLINTEGER error;
    WCHAR message[1000];
    WCHAR state[SQL_SQLSTATE_SIZE + 1];
    if (retcode == SQL_INVALID_HANDLE) {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }
    while (SQLGetDiagRec(type, handle, ++rec, state, &error, message,
        (SQLSMALLINT)(sizeof(message) / sizeof(WCHAR)), (SQLSMALLINT*)nullptr) == SQL_SUCCESS) {
        // Hide data truncated..
        if (wcsncmp(state, L"01004", 5)) {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", state, message, error);
        }
    }
}

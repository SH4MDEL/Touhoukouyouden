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
                    
                }
            }
        }
    }
}

Database::~Database()
{
    // Process data  
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void Database::DatabaseThread()
{

}

int Database::Login(const char* id, const char* password)
{
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // DB에서 데이터를 읽어 옴.
    //retcode = SQLExecDirect(hstmt, 
    //    (SQLWCHAR*)L"SELECT player_id, player_name, player_level FROM player_table WHERE player_level > 20", SQL_NTS);

    retcode = SQLExecDirect(hstmt,
        (SQLWCHAR*)L"EXEC get_high_level 20", SQL_NTS);

    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

        //// Bind columns 1, 2, and 3  
        //retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &player_id, 10, &cb_id);
        //retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szName, NAME_LEN, &cbName);
        //retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &player_level, 10, &cb_level);

        //// Fetch and print each row of data. On an error, display a message and exit.  
        //for (int i = 0; ; i++) {
        //    retcode = SQLFetch(hstmt); // 데이터를 하나씩 꺼냄
        //    if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
        //        ShowError(hstmt, SQL_HANDLE_STMT, retcode);
        //        return -1;
        //    }
        //    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
        //    {
        //        printf("%d: ID: %d, Name: %ls, Level: %d\n", i + 1, player_id, szName, player_level);
        //    }
        //    else
        //        break;
        //}
    }
    else {
        ShowError(hstmt, SQL_HANDLE_STMT, retcode);
        return -1;
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

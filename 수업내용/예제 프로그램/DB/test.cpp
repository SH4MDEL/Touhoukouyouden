// SQLConnect_ref.cpp  
// compile with: odbc32.lib  
#include <windows.h>  
#include <sqlext.h> 
#include <iostream>
using namespace std;

int main() {
    SQLHENV henv;       // 옵션 세팅
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    SQLCHAR* OutConnStr = (SQLCHAR*)malloc(255);
    SQLSMALLINT* OutConnStrLen = (SQLSMALLINT*)malloc(255);

    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);   // 5초동안 시도해서 로그인 안되면 실패

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLWCHAR*)TEXT("GameServerExample"), SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

                    // Process data  
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                        cout << "Connection OK\n";
                        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                    }
                    else {
                        cout << "Connection Fail\n";
                    }

                    SQLDisconnect(hdbc);
                }

                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                SQLBindCol();
            }

        }
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}
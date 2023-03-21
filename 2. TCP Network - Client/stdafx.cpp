#include "stdafx.h"

string	g_serverIP = "127.0.0.1";
SOCKET	g_socket;
INT		g_playerID;

namespace Move
{
	int dx[] = { 0, 1, 0, -1 };
	int dy[] = { 1, 0, -1, 0 };
}
#pragma once
#include "stdafx.h"
#include "server.h"

GameServer	g_gameServer;
SOCKET		g_serverSocket;
SOCKET		g_clientSocket;
EXP_OVER    g_expOverlapped;
mt19937		g_randomEngine{ random_device{}() };

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
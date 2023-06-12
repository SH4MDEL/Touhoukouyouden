#pragma once
#include "stdafx.h"
#include "server.h"

HANDLE      g_iocp;
GameServer	g_gameServer;
SOCKET		g_serverSocket;
SOCKET		g_clientSocket;
EXPOVERLAPPED    g_expOverlapped;
mt19937		g_randomEngine{ random_device{}() };

array<array<unordered_set<int>, W_WIDTH / (VIEW_RANGE * 2) + 1>, W_HEIGHT / (VIEW_RANGE * 2) + 1> g_sector;
array<array<mutex, W_WIDTH / (VIEW_RANGE * 2) + 1>, W_HEIGHT / (VIEW_RANGE * 2) + 1> g_sectorLock;

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
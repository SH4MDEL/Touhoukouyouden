#pragma once
#include "stdafx.h"
#include "server.h"

GameServer	g_gameServer;
SOCKET		g_serverSocket;
SOCKET		g_clientSocket;
EXP_OVER    g_expOverlapped;
mt19937		g_randomEngine{ random_device{}() };


array<array<unordered_set<int>, MAP_WIDTH / (VIEW_RANGE * 2)>, MAP_HEIGHT / (VIEW_RANGE * 2)> g_sector;

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
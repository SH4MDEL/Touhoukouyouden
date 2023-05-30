#pragma once
#include "stdafx.h"
#include "server.h"

HANDLE      g_iocp;
GameServer	g_gameServer;
SOCKET		g_serverSocket;
SOCKET		g_clientSocket;
EXP_OVER    g_expOverlapped;
mt19937		g_randomEngine{ random_device{}() };
concurrency::concurrent_priority_queue<Event> m_timerQueue;

array<array<unordered_set<int>, MAP_WIDTH / (VIEW_RANGE * 2) + 1>, MAP_HEIGHT / (VIEW_RANGE * 2) + 1> g_sector;
array<array<mutex, MAP_WIDTH / (VIEW_RANGE * 2) + 1>, MAP_HEIGHT / (VIEW_RANGE * 2) + 1> g_sectorLock;

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
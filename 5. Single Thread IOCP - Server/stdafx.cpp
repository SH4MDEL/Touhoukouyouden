#pragma once
#include "stdafx.h"
#include "server.h"

GameServer	g_gameServer;
SOCKET		g_listenSock;
mt19937		g_randomEngine{ random_device{}() };

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
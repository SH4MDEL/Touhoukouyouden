#pragma once
#include "stdafx.h"
#include "server.h"

GameServer	g_gameServer;
SOCKET		g_listenSock;
mt19937		g_randomEngine{ random_device{}() };
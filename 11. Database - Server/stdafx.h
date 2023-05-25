#pragma once
// 서버 관련 헤더파일입니다.
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#include <WS2tcpip.h>
#include <MSWSock.h>		// AcceptEx 사용을 위함.
#include "protocol.h"

// C, C++ 관련 헤더파일입니다.
#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <queue>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <memory>
#include <concurrent_priority_queue.h>
using namespace std;
#define endl "\n"

// Lua Script 사용
#include "include/lua.hpp"
#pragma comment(lib, "lua54.lib")

extern HANDLE       g_iocp;
class GameServer;
extern GameServer	g_gameServer;
extern SOCKET		g_serverSocket;
extern SOCKET		g_clientSocket;
class EXP_OVER;
extern EXP_OVER     g_expOverlapped;
extern mt19937		g_randomEngine;

constexpr int MAX_USER = 20000;
constexpr int MAX_NPC = 200000;
constexpr int VIEW_RANGE = 5;

extern array<array<unordered_set<int>, MAP_WIDTH / (VIEW_RANGE * 2) + 1>, MAP_HEIGHT / (VIEW_RANGE * 2) + 1> g_sector;
extern array<array<mutex, MAP_WIDTH / (VIEW_RANGE * 2) + 1>, MAP_HEIGHT / (VIEW_RANGE * 2) + 1> g_sectorLock;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE, OP_NPC_HELLO, OP_NPC_BYE };

namespace Move
{
    extern int dx[];
    extern int dy[];
}

namespace Utiles
{
    inline INT GetRandomINT(INT min, INT max)
    {
        uniform_int_distribution<INT> dis{ min, max };
        return dis(g_randomEngine);
    }
    inline SHORT GetRandomSHORT(SHORT min, SHORT max)
    {
        uniform_int_distribution<SHORT> dis{ min, max };
        return dis(g_randomEngine);
    }
    inline FLOAT GetRandomFLOAT(FLOAT min, FLOAT max)
    {
        uniform_real_distribution<FLOAT> dis{ min, max };
        return dis(g_randomEngine);
    }
}
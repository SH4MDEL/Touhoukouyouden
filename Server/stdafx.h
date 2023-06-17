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
#include <set>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>
#include <concurrent_unordered_map.h>
#include <random>
#include <memory>
#include <windows.h>  
#include <sqlext.h>  
#include <locale.h>
#include <fstream>
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
class EXPOVERLAPPED;
extern EXPOVERLAPPED     g_expOverlapped;
extern mt19937		g_randomEngine;

constexpr int VIEW_RANGE = 5;

extern array<array<unordered_set<int>, W_WIDTH / (VIEW_RANGE * 2) + 1>, W_HEIGHT / (VIEW_RANGE * 2) + 1> g_sector;
extern array<array<mutex, W_WIDTH / (VIEW_RANGE * 2) + 1>, W_HEIGHT / (VIEW_RANGE * 2) + 1> g_sectorLock;

enum COMP_TYPE { 
    OP_ACCEPT, OP_RECV, OP_SEND, 
    TIMER_NPC_MOVE, TIMER_NPC_ATTACK,
    DB_LOGIN_OK, DB_LOGIN_FAIL
};

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
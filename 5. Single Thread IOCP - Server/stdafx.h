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
#include <list>
#include <queue>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <random>
using namespace std;

#define endl "\n"

class GameServer;
extern GameServer	g_gameServer;
extern SOCKET		g_listenSock;
extern mt19937		g_randomEngine;

constexpr int MAX_USER = 10000;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };

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
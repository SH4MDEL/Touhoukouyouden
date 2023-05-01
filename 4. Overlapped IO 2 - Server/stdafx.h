#pragma once
// 서버 관련 헤더파일입니다.
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
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

namespace Utiles
{
    inline INT GetRandomINT(INT min, INT max)
    {
        uniform_int_distribution<INT> dis{ min, max };
        return dis(g_randomEngine);
    }
    inline FLOAT GetRandomFLOAT(FLOAT min, FLOAT max)
    {
        uniform_real_distribution<FLOAT> dis{ min, max };
        return dis(g_randomEngine);
    }
}
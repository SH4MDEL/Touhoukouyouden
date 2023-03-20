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
using namespace std;

constexpr int MAX_PLAYER = 1;
constexpr int MAP_HEIGHT = 8;
constexpr int MAP_WIDTH = 8;

#define NETWORK_DEBUG

class GameServer;
extern GameServer	g_gameServer;
extern SOCKET		g_listenSock;
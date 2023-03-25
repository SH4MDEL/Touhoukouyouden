#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
#include <atlimage.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
// C++ 런타임 헤더 파일입니다.
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
using namespace std;
// 서버 관련 헤더 파일입니다.
#include "../3. Overlapped IO - Server/protocol.h"
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#pragma comment(lib, "ws2_32.lib") // ws2_32.lib 링크

#define SHOW_CAPTIONFPS

#define MAX_TITLE 64
#define TITLESTRING TEXT("1. Game Client")

#define MAX_FPS 1.0 / 60.0

extern string g_serverIP;
extern SOCKET g_socket;
extern INT g_playerID;

namespace Move
{
	extern int dx[];
	extern int dy[];
}


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
using namespace std;

#define SHOW_CAPTIONFPS

#define MAX_TITLE 64
#define TITLESTRING TEXT("1. Game Client")

#define MAX_FPS 1.0 / 60.0

namespace Move
{
	extern int dx[];
	extern int dy[];
}
#pragma once

// Windows 헤더 파일
#include <windows.h>

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
#include <functional>
#include <sstream>
#include <fstream>
using namespace std;
#define endl "\n";

// SFML 헤더 파일입니다.
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

// 서버 관련 헤더 파일입니다.
#include "..\Server\protocol.h"

constexpr auto SCREEN_WIDTH = 16;
constexpr auto SCREEN_HEIGHT = 16;
// 1440 1040
constexpr auto TILE_WIDTH = 65;
constexpr auto PLAYER_STATUS_AREA = 400;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH + PLAYER_STATUS_AREA;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_HEIGHT * TILE_WIDTH;

class GameFramework;
extern GameFramework				g_gameFramework;
extern shared_ptr<sf::RenderWindow>	g_window;
extern sf::TcpSocket				g_socket;
extern function<void()>				g_clickEvent;
extern INT							g_clientID;
extern sf::Font						g_font;
extern INT							g_leftX;
extern INT							g_topY;
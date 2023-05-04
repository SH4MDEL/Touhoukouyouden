#include "stdafx.h"
#include "framework.h"

GameFramework					g_gameFramework{};
shared_ptr<sf::RenderWindow>	g_window;
sf::TcpSocket					g_socket{};
INT								g_clientID;
sf::Font						g_font;
INT								g_leftX;
INT								g_topY;

namespace Move
{
	int dx[] = { 0, 0, 1, -1 };
	int dy[] = { 1, -1, 0, 0 };
}
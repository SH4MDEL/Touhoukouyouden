#pragma once
#include "stdafx.h"

class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	INT RegistPlayer();
	POINT InputPlayer(INT id);

private:
	INT							m_map[MAP_HEIGHT][MAP_WIDTH];
	unordered_map<INT, POINT>	m_player;
};


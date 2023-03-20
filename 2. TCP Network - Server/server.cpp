#include "server.h"

GameServer::GameServer()
{
	memset(m_map, -1, sizeof(INT) * MAP_HEIGHT * MAP_WIDTH);
}

INT GameServer::RegistPlayer()
{
	for (int i = 0; i < MAX_PLAYER; ++i) {
		if (m_player.find(i) == m_player.end()) {
			m_player.insert({i, InputPlayer(i)});
			return i;
		}
	}
	return -1;
}

POINT GameServer::InputPlayer(INT id)
{
	for (int i = 0; i < MAP_HEIGHT; ++i) {
		for (int j = 0; j < MAP_WIDTH; ++j) {
			if (m_map[i][j] == -1) {
				m_map[i][j] = id;
				return POINT{ i, j };
			}
		}
	}
	return POINT{ -1, -1 };
}

POINT GameServer::GetPlayerPosition(INT id)
{
	if (m_player.find(id) != m_player.end()) {
		return m_player[id];
	}
	return POINT{ -1, -1 };
}

POINT GameServer::Move(INT id, POINT d)
{
	POINT from = GetPlayerPosition(id);
	POINT to;
	to.x = from.x + d.x;
	to.y = from.y + d.y;
	if (to.x >= 8 || to.x < 0 || to.y >= 8 || to.y < 0) {
		return from;
	}
	if (m_map[to.y][to.x] != -1) {
		return from;
	}
	m_map[from.y][from.x] = -1;
	m_map[to.y][to.x] = id;
	return to;
}

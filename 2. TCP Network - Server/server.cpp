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

			}
		}
	}
}

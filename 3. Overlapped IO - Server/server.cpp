#include "server.h"

GameServer::GameServer()
{
	memset(m_map, -1, sizeof(int) * MAP_HEIGHT * MAP_WIDTH);
}

int GameServer::RegistPlayer(int id)
{
	if (m_player.find(id) == m_player.end()) {
		m_player.insert({ id, InputPlayer(id)});
		return id;
	}
	return -1;
}

POINT GameServer::InputPlayer(int id)
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

POINT GameServer::GetPlayerPosition(int id)
{
	if (m_player.find(id) != m_player.end()) {
		return m_player[id];
	}
	return POINT{ -1, -1 };
}

POINT GameServer::Move(int id, POINT d)
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
	m_player[id] = to;
	return m_player[id];
}

SESSION& GameServer::GetClient(unsigned long long id)
{
	return m_clients[id];
}

unordered_map<unsigned long long, SESSION>& GameServer::GetClients()
{
	return m_clients;
}

void GameServer::RegistClient(unsigned long long id, const SOCKET& socket)
{
	m_clients.try_emplace(id, id, socket);
	m_clients[id].DoRecv();
}

void GameServer::ResetClients()
{
	m_clients.clear();
}

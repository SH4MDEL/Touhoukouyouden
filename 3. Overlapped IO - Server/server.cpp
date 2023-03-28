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

void GameServer::ExitPlayer(int id)
{
	if (m_player.find(id) != m_player.end()) {
		m_map[m_player[id].x][m_player[id].y] = -1;
		m_player.erase(id);
	}
}

POINT GameServer::InputPlayer(int id)
{
	for (int i = 0; i < MAP_WIDTH; ++i) {
		for (int j = 0; j < MAP_HEIGHT; ++j) {
			if (m_map[i][j] == -1) {
				m_map[i][j] = id;
				return POINT{ i, j };
				cout << i << ", " << j << endl;
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
	if (to.x >= MAP_WIDTH || to.x < 0 || to.y >= MAP_HEIGHT || to.y < 0) {
		return from;
	}
	if (m_map[to.x][to.y] != -1) {
		return from;
	}
	m_map[from.x][from.y] = -1;
	m_map[to.x][to.y] = id;
	m_player[id] = to;
	return m_player[id];
}

POINT GameServer::GetPlayer(int id)
{
	return m_player[id];
}

unordered_map<unsigned long long, POINT>& GameServer::GetPlayers()
{
	return m_player;
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

void GameServer::ExitClient(int id)
{
	if (m_clients.find(id) != m_clients.end()) {
		m_clients.erase(id);
	}
}

void GameServer::ResetClients()
{
	m_clients.clear();
}

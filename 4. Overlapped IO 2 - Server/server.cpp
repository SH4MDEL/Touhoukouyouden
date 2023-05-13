#include "server.h"

GameServer::GameServer()
{
}

int GameServer::RegistPlayer(UINT id)
{
	if (m_player.find(id) == m_player.end()) {
		m_player.insert({ id, {(SHORT)Utiles::GetRandomINT(0, MAP_WIDTH), (SHORT)Utiles::GetRandomINT(0, MAP_HEIGHT)} });
		return id;
	}
	return -1;
}

void GameServer::ExitPlayer(UINT id)
{
	if (m_player.find(id) != m_player.end()) {
		m_player.erase(id);
	}
}

Short2 GameServer::GetPlayerPosition(UINT id)
{
	if (m_player.find(id) != m_player.end()) {
		return m_player[id];
	}
	return Short2{ -1, -1 };
}

Short2 GameServer::Move(UINT id, UCHAR direction)
{
	Short2 from = GetPlayerPosition(id);
	short dx = (SHORT)Move::dx[direction];
	short dy = (SHORT)Move::dy[direction];
	Short2 to = { from.x + dx , from.y + dy };
	if (to.x > MAP_WIDTH || to.x < 0 || to.y > MAP_HEIGHT || to.y < 0) {
		return from;
	}
	m_player[id] = to;
	return m_player[id];
}

Short2 GameServer::GetPlayer(UINT id)
{
	return m_player[id];
}

unordered_map<UINT, Short2>& GameServer::GetPlayers()
{
	return m_player;
}

SESSION& GameServer::GetClient(UINT id)
{
	return m_clients[id];
}

unordered_map<UINT, SESSION>& GameServer::GetClients()
{
	return m_clients;
}

void GameServer::RegistClient(UINT id, const SOCKET& socket)
{
	m_clients.try_emplace(id, id, socket);
	m_clients[id].DoRecv();
}

void GameServer::ExitClient(UINT id)
{
	if (m_clients.find(id) != m_clients.end()) {
		m_clients.erase(id);
	}
}

void GameServer::ResetClients()
{
	m_clients.clear();
}

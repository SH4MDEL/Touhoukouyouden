#include "server.h"

GameServer::GameServer()
{
}

UINT GameServer::RegistClient(const SOCKET& c_socket)
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		if (!m_clients.count(i)) {
			m_clients.try_emplace(i, i, c_socket, Short2{
				Utiles::GetRandomSHORT(0, MAP_WIDTH), 
				Utiles::GetRandomSHORT(0, MAP_HEIGHT)});
			return i;
		}
	}
	return -1;
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

Short2 GameServer::GetPlayerPosition(UINT id)
{
	if (m_clients.find(id) != m_clients.end()) {
		return Short2{ m_clients[id].m_position.x, m_clients[id].m_position.y };
	}
	return Short2{ -1, -1 };
}

Short2 GameServer::Move(UINT id, Short2 d)
{
	Short2 from = GetPlayerPosition(id);
	Short2 to = from + d;
	if (to.x > MAP_WIDTH || to.x < 0 || to.y > MAP_HEIGHT || to.y < 0) {
		return from;
	}
	return m_clients[id].m_position = to;
}

CLIENT& GameServer::GetClient(UINT id)
{
	return m_clients[id];
}

unordered_map<UINT, CLIENT>& GameServer::GetClients()
{
	return m_clients;
}

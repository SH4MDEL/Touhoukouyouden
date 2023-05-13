#include "server.h"

GameServer::GameServer()
{
	for (auto& client : m_clients) {
		client = make_shared<CLIENT>();
	}
}

UINT GameServer::RegistClient(const SOCKET& c_socket)
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		{
			unique_lock<mutex> lock{ m_clients[i]->m_mutex };
			if (m_clients[i]->m_state != CLIENT::FREE) continue;
			m_clients[i]->m_state = CLIENT::ALLOC;
		}
		m_clients[i]->m_id = i;
		//m_clients[i]->m_position.x = Utiles::GetRandomINT(0, MAP_WIDTH);
		//m_clients[i]->m_position.y = Utiles::GetRandomINT(0, MAP_HEIGHT);
		m_clients[i]->m_position.x = rand() % (MAP_WIDTH);
		m_clients[i]->m_position.y = rand() % (MAP_HEIGHT);
		m_clients[i]->m_name[0] = 0;
		m_clients[i]->m_prevRemain = 0;
		m_clients[i]->m_socket = c_socket;
		return i;
	}
	return -1;
}

void GameServer::ExitClient(UINT id)
{
	closesocket(m_clients[id]->m_socket);
	lock_guard<mutex> lock{ m_clients[id]->m_mutex };
	m_clients[id]->m_state = CLIENT::FREE;
}

Short2 GameServer::GetPlayerPosition(UINT id)
{
	return Short2{ m_clients[id]->m_position.x, m_clients[id]->m_position.y };
}

Short2 GameServer::Move(UINT id, UCHAR direction)
{
	Short2 from = GetPlayerPosition(id);
	auto dx = Move::dx[direction];
	auto dy = Move::dy[direction];
	Short2 to = { from.x + (SHORT)dx , from.y + (SHORT)dy};
	if (to.x > MAP_WIDTH || to.x < 0 || to.y > MAP_HEIGHT || to.y < 0) {
		return from;
	}
	return m_clients[id]->m_position = to;
}

shared_ptr<CLIENT> GameServer::GetClient(UINT id)
{
	return m_clients[id];
}

array<shared_ptr<CLIENT>, MAX_USER>& GameServer::GetClients()
{
	return m_clients;
}

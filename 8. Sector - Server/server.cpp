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

		g_sectorLock[m_clients[i]->m_position.y / (VIEW_RANGE * 2)][m_clients[i]->m_position.x / (VIEW_RANGE * 2)].lock();
		g_sector[m_clients[i]->m_position.y / (VIEW_RANGE * 2)][m_clients[i]->m_position.x / (VIEW_RANGE * 2)].insert(i);
		g_sectorLock[m_clients[i]->m_position.y / (VIEW_RANGE * 2)][m_clients[i]->m_position.x / (VIEW_RANGE * 2)].unlock();

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

BOOL GameServer::CanSee(UINT id1, UINT id2)
{
	if (std::abs(m_clients[id1]->m_position.x - m_clients[id2]->m_position.x) > VIEW_RANGE) return false;
	return std::abs(m_clients[id1]->m_position.y - m_clients[id2]->m_position.y) <= VIEW_RANGE;
}

void GameServer::Move(UINT id, UCHAR direction)
{
	Short2 from = GetPlayerPosition(id);
	auto dx = Move::dx[direction];
	auto dy = Move::dy[direction];
	Short2 to = { from.x + (SHORT)dx , from.y + (SHORT)dy};
	if (to.x > MAP_WIDTH || to.x < 0 || to.y > MAP_HEIGHT || to.y < 0) {
		return;
	}
	m_clients[id]->m_position = to;

	// 만약 섹터의 위치가 바뀌었을 경우
	if ((from.x / (VIEW_RANGE * 2) != to.x / (VIEW_RANGE * 2)) ||
		(from.y / (VIEW_RANGE * 2) != to.y / (VIEW_RANGE * 2))) {
		// 데드락을 방지하기 위해 락을 거는 순서를 정한다.
		priority_queue<Short2> pq;
		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].lock();
		}
		// 이전 섹터에서 오브젝트의 ID를 삭제한다.
		g_sector[from.y / (VIEW_RANGE * 2)][from.x / (VIEW_RANGE * 2)].erase(id);
		// 새로운 섹터에 오브젝트의 ID를 삽입한다.
		g_sector[to.y / (VIEW_RANGE * 2)][to.x / (VIEW_RANGE * 2)].insert(id);

		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].unlock();
		}
	}
}

shared_ptr<CLIENT> GameServer::GetClient(UINT id)
{
	return m_clients[id];
}

array<shared_ptr<CLIENT>, MAX_USER>& GameServer::GetClients()
{
	return m_clients;
}

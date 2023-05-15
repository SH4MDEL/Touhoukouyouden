#include "server.h"

GameServer::GameServer()
{
	for (auto& client : m_clients) {
		client = make_shared<CLIENT>();
	}
	for (auto& npc : m_npcs) {
		npc = make_shared<NPC>();
	}
}

void GameServer::InitializeNPC()
{
	for (int i = MAX_USER; auto & npc : m_npcs) {
		npc->m_id = i++;
		//npc->m_position.x = rand() % (MAP_WIDTH);
		//npc->m_position.y = rand() % (MAP_HEIGHT);
		npc->m_position.x = rand() % (MAP_WIDTH);
		npc->m_position.y = rand() % (MAP_HEIGHT);
		npc->m_name[0] = 0;
		npc->m_state = OBJECT::INGAME;

		// 싱글쓰레드에서 초기화하므로 락을 걸 필요가 없다.
		g_sector[npc->m_position.y / (VIEW_RANGE * 2)][npc->m_position.x / (VIEW_RANGE * 2)].insert(npc->m_id);
	}
}

UINT GameServer::RegistClient(const SOCKET& c_socket)
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		{
			unique_lock<mutex> lock{ m_clients[i]->m_mutex };
			if (m_clients[i]->m_state != OBJECT::FREE) continue;
			m_clients[i]->m_state = OBJECT::ALLOC;
		}
		m_clients[i]->m_id = i;
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
	{
		unique_lock<mutex> stateLock{ m_clients[id]->m_mutex };
		m_clients[id]->m_state = CLIENT::FREE;
	}
	unique_lock<mutex> viewLock{ m_clients[id]->m_viewLock };
	m_clients[id]->m_viewList.clear();
}

Short2 GameServer::GetPlayerPosition(UINT id)
{
	return Short2{ m_clients[id]->m_position.x, m_clients[id]->m_position.y };
}

BOOL GameServer::CanSee(UINT id1, UINT id2)
{
	Short2 position1; Short2 position2;
	if (id1 < MAX_USER) position1 = m_clients[id1]->m_position;
	else position1 = m_npcs[id1 - MAX_USER]->m_position;
	if (id2 < MAX_USER) position2 = m_clients[id2]->m_position;
	else position2 = m_npcs[id2 - MAX_USER]->m_position;

	if (std::abs(position1.x - position2.x) > VIEW_RANGE) return false;
	return std::abs(position1.y - position2.y) <= VIEW_RANGE;
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

shared_ptr<NPC> GameServer::GetNPC(UINT id)
{
	return m_npcs[id - MAX_USER];
}

void GameServer::AddTimer(UINT id, Event::Type type, chrono::system_clock::time_point executeTime)
{
	unique_lock<mutex> timerLock{ m_timerLock };
	m_timerQueue.push( Event{id, type, executeTime} );
}

void GameServer::WakeupNPC(UINT id)
{
	//printf("Wakeup id : %d\n", id);
	AddTimer(id, Event::RANDOM_MOVE, chrono::system_clock::now() + 1s);
}

void GameServer::MoveNPC(UINT id)
{
	unordered_set<int> viewList;
	{
		short sectorX = m_npcs[id - MAX_USER]->m_position.x / (VIEW_RANGE * 2);
		short sectorY = m_npcs[id - MAX_USER]->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// 주변 9개의 섹터 전부 조사
		for (int i = 0; i < 9; ++i) {
			if (sectorX + dx[i] >= MAP_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= MAP_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
				continue;
			}

			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
			for (auto& cid : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
				if (cid >= MAX_USER) continue;
				if (m_clients[cid]->m_state != OBJECT::INGAME) continue;
				if (CanSee(id, cid)) viewList.insert(cid);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// 이동 처리
	Short2 from = m_npcs[id - MAX_USER]->m_position;
	Short2 d;
	switch (rand() % 4)
	{
	case 0: d.x = 0; d.y = 1; break;
	case 1: d.x = 0; d.y = -1; break;
	case 2: d.x = 1; d.y = 0; break;
	case 3: d.x = -1; d.y = 0; break;
	}
	
	Short2 to = { from + d };
	if (to.x > MAP_WIDTH || to.x < 0 || to.y > MAP_HEIGHT || to.y < 0) {
		return;
	}
	m_npcs[id - MAX_USER]->m_position = to;

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


	unordered_set<int> newViewList;
	{
		short sectorX = m_npcs[id - MAX_USER]->m_position.x / (VIEW_RANGE * 2);
		short sectorY = m_npcs[id - MAX_USER]->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// 주변 9개의 섹터 전부 조사
		for (int i = 0; i < 9; ++i) {
			if (sectorX + dx[i] >= MAP_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= MAP_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
				continue;
			}

			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
			for (auto& cid : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
				if (cid >= MAX_USER) continue;
				if (m_clients[cid]->m_state != OBJECT::INGAME) continue;
				if (CanSee(id, cid)) newViewList.insert(cid);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// 새 View List에 추가되었는데 이전에 없던 플레이어의 정보 추가
	for (auto& cid : newViewList) {
		m_clients[cid]->m_viewLock.lock();
		if (!m_clients[cid]->m_viewList.count(id)) {
			// View List에 없다면 더해준다.
			m_clients[cid]->m_viewLock.unlock();
			m_clients[cid]->SendAddPlayer(id);
		}
		else {
			// 있다면 이동 시킨다.
			m_clients[cid]->m_viewLock.unlock();
			m_clients[cid]->SendObjectInfo(id);
		}
	}

	// 새 View List에선 제거되었는데 이전에 있던 플레이어의 정보 삭제
	for (auto& cid : viewList) {
		if (!newViewList.count(cid)) {
			m_clients[cid]->SendExitPlayer(id);
		}
	}
}

void GameServer::TimerThread(HANDLE hiocp)
{
	while (true)
	{
		m_timerLock.lock();
		if (m_timerQueue.empty()) {
			m_timerLock.unlock();
			this_thread::sleep_for(10ms);
			continue;
		}
		auto timerEvent = m_timerQueue.top();
		if (timerEvent.m_executeTime > chrono::system_clock::now()) {
			m_timerLock.unlock();
			this_thread::sleep_for(10ms);
			continue;
		}
		m_timerQueue.pop();

		m_timerLock.unlock();
		EXP_OVER* over = new EXP_OVER;
		over->m_compType = COMP_TYPE::OP_NPC;
		// type 제외하고 초기화할 필요 없음
		PostQueuedCompletionStatus(hiocp, 1, timerEvent.m_id, &over->m_overlapped);
	}
}
#include "server.h"

GameServer::GameServer()
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		m_objects[i] = make_shared<CLIENT>();
	}
	for (UINT i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		m_objects[i] = make_shared<NPC>();
	}
}

void GameServer::InitializeNPC()
{
	for (UINT i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		m_objects[i]->m_id = i;
		m_objects[i]->m_position.x = rand() % (MAP_WIDTH);
		m_objects[i]->m_position.y = rand() % (MAP_HEIGHT);
		sprintf_s(m_objects[i]->m_name, "N%d", i);

		m_objects[i]->m_state = OBJECT::INGAME;

		// 싱글쓰레드에서 초기화하므로 락을 걸 필요가 없다.
		g_sector[m_objects[i]->m_position.y / (VIEW_RANGE * 2)][m_objects[i]->m_position.x / (VIEW_RANGE * 2)].insert(m_objects[i]->m_id);
	}
}

UINT GameServer::RegistClient(const SOCKET& c_socket)
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		auto client = static_pointer_cast<CLIENT>(m_objects[i]);
		{
			unique_lock<mutex> lock{ client->m_mutex };
			if (client->m_state != OBJECT::FREE) continue;
			client->m_state = OBJECT::ALLOC;
		}
		client->m_id = i;
		client->m_position.x = rand() % (MAP_WIDTH);
		client->m_position.y = rand() % (MAP_HEIGHT);
		client->m_name[0] = 0;
		client->m_prevRemain = 0;
		client->m_socket = c_socket;

		g_sectorLock[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].lock();
		g_sector[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].insert(i);
		g_sectorLock[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].unlock();

		return i;
	}
	return -1;
}

void GameServer::ExitClient(UINT id)
{
	auto exitClient = static_pointer_cast<CLIENT>(m_objects[id]);

	g_sectorLock[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].lock();
	g_sector[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].erase(id);
	g_sectorLock[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].unlock();

	for (UINT i = 0; i < MAX_USER; ++i) {
		auto client = static_pointer_cast<CLIENT>(m_objects[i]);
		{
			unique_lock<mutex> lock(client->m_mutex);
			if (client->m_state != OBJECT::INGAME) continue;
		}
		if (client->m_id == id) continue;
		client->SendExitPlayer(id);
	}

	closesocket(exitClient->m_socket);
	{
		unique_lock<mutex> stateLock{ exitClient->m_mutex };
		exitClient->m_state = CLIENT::FREE;
	}
	unique_lock<mutex> viewLock{ exitClient->m_viewLock };
	exitClient->m_viewList.clear();
}

Short2 GameServer::GetPlayerPosition(UINT id)
{
	return Short2{ m_objects[id]->m_position.x, m_objects[id]->m_position.y };
}

BOOL GameServer::CanSee(UINT id1, UINT id2)
{
	if (std::abs(m_objects[id1]->m_position.x - m_objects[id2]->m_position.x) > VIEW_RANGE) return false;
	return std::abs(m_objects[id1]->m_position.y - m_objects[id2]->m_position.y) <= VIEW_RANGE;
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
	m_objects[id]->m_position = to;

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
	return static_pointer_cast<CLIENT>(m_objects[id]);
}

shared_ptr<NPC> GameServer::GetNPC(UINT id)
{
	return static_pointer_cast<NPC>(m_objects[id]);
}

void GameServer::AddTimer(UINT id, Event::Type type, chrono::system_clock::time_point executeTime)
{
	m_timerQueue.push( Event{id, type, executeTime} );
}

void GameServer::WakeupNPC(UINT id)
{
	//printf("Wakeup id : %d\n", id);
	AddTimer(id, Event::RANDOM_MOVE, chrono::system_clock::now() + 1s);
}

void GameServer::MoveNPC(UINT id)
{
	auto npc = static_pointer_cast<NPC>(m_objects[id]);
	unordered_set<int> viewList;
	{
		short sectorX = npc->m_position.x / (VIEW_RANGE * 2);
		short sectorY = npc->m_position.y / (VIEW_RANGE * 2);
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
				if (m_objects[cid]->m_state != OBJECT::INGAME) continue;
				if (CanSee(id, cid)) viewList.insert(cid);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// 이동 처리
	Short2 from = npc->m_position;
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
	npc->m_position = to;

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
		short sectorX = npc->m_position.x / (VIEW_RANGE * 2);
		short sectorY = npc->m_position.y / (VIEW_RANGE * 2);
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
				if (npc->m_state != OBJECT::INGAME) continue;
				if (CanSee(id, cid)) newViewList.insert(cid);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// 새 View List에 추가되었는데 이전에 없던 플레이어의 정보 추가
	for (auto& cid : newViewList) {
		auto client = static_pointer_cast<CLIENT>(m_objects[cid]);
		client->m_viewLock.lock();
		if (!client->m_viewList.count(id)) {
			// View List에 없다면 더해준다.
			client->m_viewLock.unlock();
			client->SendAddPlayer(id);
		}
		else {
			// 있다면 이동 시킨다.
			client->m_viewLock.unlock();
			client->SendObjectInfo(id);
		}
	}

	// 새 View List에선 제거되었는데 이전에 있던 플레이어의 정보 삭제
	for (auto& cid : viewList) {
		auto client = static_pointer_cast<CLIENT>(m_objects[cid]);
		if (!newViewList.count(cid)) {
			client->SendExitPlayer(id);
		}
	}
}

void GameServer::TimerThread(HANDLE hiocp)
{
	while (true)
	{
		auto currentTime = chrono::system_clock::now();
		Event ev;
		if (m_timerQueue.try_pop(ev)) {
			if (ev.m_executeTime > currentTime) {
				m_timerQueue.push(ev);
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.m_type)
			{
			case Event::RANDOM_MOVE:
				EXP_OVER* over = new EXP_OVER;
				over->m_compType = COMP_TYPE::OP_NPC;
				// type 제외하고 초기화할 필요 없음
				PostQueuedCompletionStatus(hiocp, 1, ev.m_id, &over->m_overlapped);
				break;
			}
			continue;
		}
		this_thread::sleep_for(1ms);
	}
}
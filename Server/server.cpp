#include "server.h"

INT API_SendMessage(lua_State* state) { return g_gameServer.Lua_SendMessage(state); }
INT API_GetX(lua_State* state) { return g_gameServer.Lua_GetX(state); }
INT API_GetY(lua_State* state) { return g_gameServer.Lua_GetY(state); }

GameServer::GameServer()
{
	for (UINT i = 0; i < MAX_USER; ++i) {
		m_objects[i] = make_shared<CLIENT>();
	}
	for (UINT i = MAX_USER; i < MAX_USER + MAX_MONSTER; ++i) {
		m_objects[i] = make_shared<NPC>();
	}
}

void GameServer::LoadMap()
{
	cout << "Load Map begin\n";
	ifstream in("map.txt", ios::binary);

	for (int i = 0; i < W_HEIGHT; ++i) {
		for (int j = 0; j < W_WIDTH; ++j) {
			in >> m_map[i][j];
		}
	}
	cout << "Load Map end\n";
}

void GameServer::InitializeMonster()
{
	cout << "Initialize Monster begin\n";
	for (UINT i = MAX_USER; i < MAX_USER + MAX_MONSTER; ++i) {
		auto npc = static_pointer_cast<NPC>(m_objects[i]);
		npc->m_id = i;
		npc->m_serial = Serial::Monster::SHROOM;
		const auto setting = Setting::GetInstance().GetMonsterSetting(npc->m_serial);

		do 
		{
			npc->m_position.x = rand() % (W_WIDTH);
			npc->m_position.y = rand() % (W_HEIGHT);
		} while ((m_map[npc->m_position.y][npc->m_position.x] & TileInfo::BLOCKING) ||
			VillageBorder::InVillage({ npc->m_position.x, npc->m_position.y }));

		strcpy_s(npc->m_name, setting.name.c_str());
		npc->m_level = setting.level;
		npc->m_exp = setting.exp;
		npc->m_hp = setting.hp;
		npc->m_maxHp = setting.hp;
		npc->m_speed = setting.speed;
		npc->m_atk = setting.atk;
		npc->m_type = setting.type;

		npc->m_state = OBJECT::INGAME;

		// 싱글쓰레드에서 초기화하므로 락을 걸 필요가 없다.
		g_sector[npc->m_position.y / (VIEW_RANGE * 2)][npc->m_position.x / (VIEW_RANGE * 2)].insert(npc->m_id);

		npc->m_luaState = luaL_newstate();
		lua_gc(npc->m_luaState, LUA_GCSTOP);

		luaL_openlibs(npc->m_luaState);
		luaL_loadfile(npc->m_luaState, "npc.lua");
		lua_pcall(npc->m_luaState, 0, 0, 0);

		lua_getglobal(npc->m_luaState, "set_uid");
		lua_pushnumber(npc->m_luaState, npc->m_id);
		lua_pcall(npc->m_luaState, 1, 0, 0);
		lua_pop(npc->m_luaState, 1);


		lua_register(npc->m_luaState, "API_SendMessage", API_SendMessage);
		lua_register(npc->m_luaState, "API_GetX", API_GetX);
		lua_register(npc->m_luaState, "API_GetY", API_GetY);

	}
	cout << "Initialize Monster end\n";
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
		client->m_serial = -1;
		client->m_position.x = -1;
		client->m_position.y = -1;
		client->m_name[0] = 0;
		client->m_prevRemain = 0;
		client->m_socket = c_socket;

		return i;
	}
	return -1;
}

void GameServer::InputClient(UINT id, int serial, Short2 position, int level, int exp, int hp, int maxHp)
{
	auto client = static_pointer_cast<CLIENT>(m_objects[id]);

	client->m_serial = serial;
	client->m_position = position;
	client->m_level = level;
	client->m_exp = exp;
	client->m_hp = hp;
	client->m_maxHp = maxHp;

	g_sectorLock[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].lock();
	g_sector[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].insert(id);
	g_sectorLock[client->m_position.y / (VIEW_RANGE * 2)][client->m_position.x / (VIEW_RANGE * 2)].unlock();
}

void GameServer::ExitClient(UINT id)
{
	auto exitClient = static_pointer_cast<CLIENT>(m_objects[id]);

	g_sectorLock[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].lock();
	g_sector[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].erase(id);
	g_sectorLock[exitClient->m_position.y / (VIEW_RANGE * 2)][exitClient->m_position.x / (VIEW_RANGE * 2)].unlock();

	Database::GetInstance().AddDatabaseEvent(DatabaseEvent{
		(UINT)exitClient->m_id, DatabaseEvent::Type::LOGOUT, 
		DataBaseUserInfo{"","", exitClient->m_position.x, exitClient->m_position.y }
		});

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

BOOL GameServer::IsSamePosition(UINT id1, UINT id2)
{
	return (m_objects[id1]->m_position.x == m_objects[id2]->m_position.x &&
		m_objects[id1]->m_position.y == m_objects[id2]->m_position.y);
}

void GameServer::Move(UINT id, UCHAR direction)
{
	Short2 from = GetPlayerPosition(id);
	auto dx = Move::dx[direction];
	auto dy = Move::dy[direction];
	Short2 to = { from.x + (SHORT)dx , from.y + (SHORT)dy};
	if (to.x > W_WIDTH || to.x < 0 || to.y > W_HEIGHT || to.y < 0) {
		return;
	}
	if (m_map[to.y][to.x] & TileInfo::BLOCKING) return;
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

// 자고 있는 NPC를 깨운다.
// NPC의 성질에 따라 다른 상태를 적용해 줄 필요가 있다.
void GameServer::WakeupNPC(UINT id, UINT waker)
{
	EXPOVERLAPPED* expOverlapped = new EXPOVERLAPPED;
	expOverlapped->m_compType = TIMER_NPC_HELLO;
	memcpy(expOverlapped->m_sendMsg, &waker, sizeof(waker));
	PostQueuedCompletionStatus(g_iocp, 1, id, &expOverlapped->m_overlapped);

	// 이미 활동중인 NPC라면 return
	auto npc = static_pointer_cast<NPC>(m_objects[id]);
	if (npc->m_isActive) return;
	bool oldState = false;
	if (!atomic_compare_exchange_strong(&npc->m_isActive, &oldState, true)) return;

	// 아니라면 타이머 쓰레드에 이동 명령 통지
	Timer::GetInstance().AddTimerEvent(id, TimerEvent::ROAMING, chrono::system_clock::now() + 1s, 0, waker);
}

void GameServer::SleepNPC(UINT id)
{
	auto npc = static_pointer_cast<NPC>(m_objects[id]);
	if (!npc->m_isActive) return;
	bool oldState = true;
	atomic_compare_exchange_strong(&npc->m_isActive, &oldState, false);
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
			if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
				continue;
			}

			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
			for (auto& cid : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
				if (cid >= MAX_USER) continue;
				if (m_objects[cid]->m_state != OBJECT::INGAME) continue;
				if (CanSee(id, cid)) viewList.insert(cid);
				// 섹터 내의 오브젝트 중 시야 안의 오브젝트만 뷰 리스트에 추가
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
	if (to.x > W_WIDTH || to.x < 0 || to.y > W_HEIGHT || to.y < 0) {
		return;
	}
	if (m_map[to.y][to.x] & TileInfo::BLOCKING) return;
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
			if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
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

INT GameServer::Lua_GetX(lua_State* state)
{
	int id = (int)lua_tointeger(state, -1);
	lua_pop(state, 2);
	int x = m_objects[id]->m_position.x;
	lua_pushnumber(state, x);
	return 1;
}

INT GameServer::Lua_GetY(lua_State* state)
{
	int id = (int)lua_tointeger(state, -1);
	lua_pop(state, 2);
	int y = m_objects[id]->m_position.y;
	lua_pushnumber(state, y);
	return 1;
}

int GameServer::Lua_SendMessage(lua_State* state)
{
	int npc = (int)lua_tointeger(state, -3);
	int user = (int)lua_tointeger(state, -2);
	char* message = (char*)lua_tostring(state, -1);
	lua_pop(state, 4);

	auto client = static_pointer_cast<CLIENT>(m_objects[user]);
	client->SendChat(npc, message);
	return 0;
}

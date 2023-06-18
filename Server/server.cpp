#include "server.h"

INT API_GetX(lua_State* state) { return g_gameServer.Lua_GetX(state); }
INT API_GetY(lua_State* state) { return g_gameServer.Lua_GetY(state); }
int API_SendMessage(lua_State* state) { return g_gameServer.Lua_SendMessage(state); }
int API_AStar(lua_State* state) { return g_gameServer.Lua_AStar(state); }

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
		auto randomInt = Utiles::GetRandomINT(1, 10);
		if (randomInt == 1) npc->m_serial = Serial::Monster::RIBBONPIG;
		if (randomInt >= 2 && randomInt < 5) npc->m_serial = Serial::Monster::MUSHROOM;
		if (randomInt >= 5 && randomInt <= 10) npc->m_serial = Serial::Monster::SHROOM;

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
		npc->m_moveType = setting.moveType;
		npc->m_waitType = setting.waitType;

		npc->m_state = OBJECT::LIVE;

		// 싱글쓰레드에서 초기화하므로 락을 걸 필요가 없다.
		g_sector[npc->m_position.y / (VIEW_RANGE * 2)][npc->m_position.x / (VIEW_RANGE * 2)].insert(npc->m_id);

		npc->m_monsterState = luaL_newstate();
		lua_gc(npc->m_monsterState, LUA_GCSTOP);

		luaL_openlibs(npc->m_monsterState);
		luaL_loadfile(npc->m_monsterState, "ai.lua");
		lua_pcall(npc->m_monsterState, 0, 0, 0);

		lua_getglobal(npc->m_monsterState, "set_uid");
		lua_pushnumber(npc->m_monsterState, npc->m_id);
		lua_pcall(npc->m_monsterState, 1, 0, 0);
		lua_pop(npc->m_monsterState, 1);

		lua_register(npc->m_monsterState, "API_GetX", API_GetX);
		lua_register(npc->m_monsterState, "API_GetY", API_GetY);
		lua_register(npc->m_monsterState, "API_SendMessage", API_SendMessage);
		lua_register(npc->m_monsterState, "API_AStar", API_AStar);

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

	const auto setting = Setting::GetInstance().GetCharacterSetting(client->m_serial);

	client->m_baseAtk = setting.baseAtk;
	client->m_bonusAtk = setting.bonusAtk;
	client->m_baseSkill = setting.baseSkill;
	client->m_bonusSkill = setting.bonusSkill;

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
			if (!(client->m_state & OBJECT::INGAME)) continue;
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

void GameServer::Teleport(UINT id, Short2 position)
{
	Short2 from = GetPlayerPosition(id);
	Short2 to = position;
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

void GameServer::Attack(UINT id, UCHAR direction)
{
	Short2 from = GetPlayerPosition(id);
	auto dx = Move::dx[direction];
	auto dy = Move::dy[direction];
	Short2 to = { from.x + (SHORT)dx , from.y + (SHORT)dy };
	if (to.x > W_WIDTH || to.x < 0 || to.y > W_HEIGHT || to.y < 0) {
		return;
	}
	if (m_map[to.y][to.x] & TileInfo::BLOCKING) return;


	unordered_set<int> monsterList;
	// 공격 목표에 해당하는 섹터 하나만 조사 (공격 범위는 1칸이므로)
	short sectorX = to.x / (VIEW_RANGE * 2);
	short sectorY = to.y / (VIEW_RANGE * 2);
	g_sectorLock[sectorY][sectorX].lock();
	for (auto& cid : g_sector[sectorY][sectorX]) {
		if (cid < MAX_USER) continue;
		if (!(m_objects[cid]->m_state & OBJECT::LIVE)) continue;
		if (m_objects[cid]->m_position == to) {
			monsterList.insert(cid);
		}
	}
	g_sectorLock[sectorY][sectorX].unlock();

	for (auto monster : monsterList) {
		m_objects[monster]->Attacked(id);
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

// 자고 있는 몬스터를 깨운다.
// 이 함수는 플레이어의 시야에 몬스터가 들어왔을 때 호출된다.
void GameServer::WakeupNPC(UINT id, UINT waker)
{
	// 이미 활동중인 NPC라면 return
	auto& monster = static_pointer_cast<NPC>(m_objects[id]);
	if (monster->m_isActive) return;
	bool oldState = false;
	if (!atomic_compare_exchange_strong(&monster->m_isActive, &oldState, true)) return;

	Timer::GetInstance().AddTimerEvent(id, TimerEvent::MOVE, 
		chrono::system_clock::now() + monster->m_speed, 0, waker);

	Timer::GetInstance().AddTimerEvent(id, TimerEvent::ATTACK,
		chrono::system_clock::now() + 1s, 0, -1);
}

void GameServer::SleepNPC(UINT id)
{
	auto npc = static_pointer_cast<NPC>(m_objects[id]);
	if (!npc->m_isActive) return;
	bool oldState = true;
	atomic_compare_exchange_strong(&npc->m_isActive, &oldState, false);
}

void GameServer::RoamingNPC(UINT id)
{
	auto npc = static_pointer_cast<NPC>(m_objects[id]);
	if (!(npc->m_state & OBJECT::LIVE)) return;

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
				if (!(m_objects[cid]->m_state & OBJECT::INGAME)) continue;
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
				if (!(npc->m_state & OBJECT::INGAME)) continue;
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
			client->SendMoveObject(id);
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

void GameServer::PathfindingNPC(UINT id, UINT target)
{
	auto& monster = g_gameServer.GetNPC(id);
	if (!(monster->m_state & OBJECT::LIVE)) return;
	if (!(g_gameServer.GetClient(target)->m_state & OBJECT::LIVE)) return;
	auto& targetPosition = g_gameServer.GetClient(target)->m_position;
	if (monster->m_position == targetPosition) return;

	monster->m_luaLock.lock();
	lua_getglobal(monster->m_monsterState, "pathfinding");
	lua_pushnumber(monster->m_monsterState, target);
	lua_pcall(monster->m_monsterState, 1, 1, 0);

	int result = lua_tointeger(monster->m_monsterState, -1);
	lua_pop(monster->m_monsterState, 1);
	monster->m_luaLock.unlock();

	if (result == -1) {
		// 이동 실패 처리
		cout << "이동 불가능" << endl;
		return;
	}

	unordered_set<int> viewList;
	{
		short sectorX = monster->m_position.x / (VIEW_RANGE * 2);
		short sectorY = monster->m_position.y / (VIEW_RANGE * 2);
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
				if (!(m_objects[cid]->m_state & OBJECT::INGAME)) continue;
				if (CanSee(id, cid)) viewList.insert(cid);
				// 섹터 내의 오브젝트 중 시야 안의 오브젝트만 뷰 리스트에 추가
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// 이동 처리
	Short2 from = monster->m_position;
	Short2 d;
	switch (result)
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
	monster->m_position = to;

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
		short sectorX = monster->m_position.x / (VIEW_RANGE * 2);
		short sectorY = monster->m_position.y / (VIEW_RANGE * 2);
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
				if (!(monster->m_state & OBJECT::INGAME)) continue;
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
			client->SendMoveObject(id);
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

int GameServer::Lua_AStar(lua_State* state)
{
	int finder = lua_tonumber(state, -2);
	int target = lua_tonumber(state, -1);
	lua_pop(state, 2);

	Short2 start = m_objects[finder]->m_position;
	Short2 end = m_objects[target]->m_position;

	// AStar 구현
	// pair.first = 휴리스틱
	// pair.second = 해당 좌표
	priority_queue<pair<int, Short2>, vector<pair<int, Short2>>, greater<pair<int, Short2>>> pq;
	unordered_map<Short2, int> open;
	unordered_map<Short2, Short2> parent;
	// 이미 탐색한 노드(닫힌 노드)의 목록
	unordered_set<Short2> closed;

	// 출발 노드에서 도착 노드까지의 예상 비용을 구한다.
	int s_h = abs(start.x - end.x) + abs(start.y - end.y);
	if (s_h == 1) {
		if (start.x == end.x && start.y + 1 == end.y) lua_pushnumber(state, 0);
		if (start.x == end.x && start.y - 1 == end.y) lua_pushnumber(state, 1);
		if (start.x + 1 == end.x && start.y == end.y) lua_pushnumber(state, 2);
		if (start.x - 1 == end.x && start.y == end.y) lua_pushnumber(state, 3);
		return 1;
	}
	pq.push({ s_h, start });
	open.insert({ start, s_h });
	parent.insert({ start, Short2{-1, -1} });


	while (!pq.empty()) {
		pair<int, Short2> top = pq.top(); pq.pop();
		open.erase(top.second);
		closed.insert(top.second);

		// 인접 4칸의 좌표를 조사한다.
		const array<Short2, 4> d = { Short2{0, 1},Short2{0, -1},Short2{1, 0} ,Short2{-1, 0} };
		for (int i = 0; i < 4; ++i) {
			const Short2 to = { top.second + d[i]};
			// 해당 칸으로 이동 불가능하면 큐에 추가 X
			if (m_map[to.y][to.x] & TileInfo::BLOCKING) {
				continue;
			}
			// 해당 노드를 이미 방문했으면 큐에 추가 X
			if (closed.find(to) != closed.end()) {
				continue;
			}
			if (to.x > W_WIDTH || to.x < 0 || to.y > W_HEIGHT || to.y < 0) continue;

			// 목적지 탐색 성공
			if (to.x == end.x && to.y == end.y) {
				Short2 p = top.second;
				while (parent[p].x != start.x || parent[p].y != start.y) {
					p = parent[p];
				}
				if (start.x == p.x && start.y + 1 == p.y) lua_pushnumber(state, 0);
				if (start.x == p.x && start.y - 1 == p.y) lua_pushnumber(state, 1);
				if (start.x + 1 == p.x && start.y == p.y) lua_pushnumber(state, 2);
				if (start.x - 1 == p.x && start.y == p.y) lua_pushnumber(state, 3);
				return 1;
			}

			// 휴리스틱 : 장애물이 없다고 가정한 목적지까지의 거리
			int h = abs(to.x - end.x) + abs(to.y - end.y);
			// g : 해당 노드로 이동하기 위한 비용
			int g = top.first + 1;
			int f = h + g;

			// 만약 열린 노드에 해당 노드가 있으면 비용이 더 낮을 경우에만 갱신
			if (open.count(to)) {
				if (open[to] > f) {
					open[to] = f;
				}
				else continue;
			}
			else open.insert({ to, f });
			pq.push({f, to});
			parent.insert({ to, top.second });
		}
	}

	// 목표 탐색 실패
	lua_pushnumber(state, -1);
	return 1;
}


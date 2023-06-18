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

		// �̱۾����忡�� �ʱ�ȭ�ϹǷ� ���� �� �ʿ䰡 ����.
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

	// ���� ������ ��ġ�� �ٲ���� ���
	if ((from.x / (VIEW_RANGE * 2) != to.x / (VIEW_RANGE * 2)) ||
		(from.y / (VIEW_RANGE * 2) != to.y / (VIEW_RANGE * 2))) {
		// ������� �����ϱ� ���� ���� �Ŵ� ������ ���Ѵ�.
		priority_queue<Short2> pq;
		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].lock();
		}
		// ���� ���Ϳ��� ������Ʈ�� ID�� �����Ѵ�.
		g_sector[from.y / (VIEW_RANGE * 2)][from.x / (VIEW_RANGE * 2)].erase(id);
		// ���ο� ���Ϳ� ������Ʈ�� ID�� �����Ѵ�.
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

	// ���� ������ ��ġ�� �ٲ���� ���
	if ((from.x / (VIEW_RANGE * 2) != to.x / (VIEW_RANGE * 2)) ||
		(from.y / (VIEW_RANGE * 2) != to.y / (VIEW_RANGE * 2))) {
		// ������� �����ϱ� ���� ���� �Ŵ� ������ ���Ѵ�.
		priority_queue<Short2> pq;
		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].lock();
		}
		// ���� ���Ϳ��� ������Ʈ�� ID�� �����Ѵ�.
		g_sector[from.y / (VIEW_RANGE * 2)][from.x / (VIEW_RANGE * 2)].erase(id);
		// ���ο� ���Ϳ� ������Ʈ�� ID�� �����Ѵ�.
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
	// ���� ��ǥ�� �ش��ϴ� ���� �ϳ��� ���� (���� ������ 1ĭ�̹Ƿ�)
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

// �ڰ� �ִ� ���͸� �����.
// �� �Լ��� �÷��̾��� �þ߿� ���Ͱ� ������ �� ȣ��ȴ�.
void GameServer::WakeupNPC(UINT id, UINT waker)
{
	// �̹� Ȱ������ NPC��� return
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
		// �ֺ� 9���� ���� ���� ����
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
				// ���� ���� ������Ʈ �� �þ� ���� ������Ʈ�� �� ����Ʈ�� �߰�
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// �̵� ó��
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

	// ���� ������ ��ġ�� �ٲ���� ���
	if ((from.x / (VIEW_RANGE * 2) != to.x / (VIEW_RANGE * 2)) ||
		(from.y / (VIEW_RANGE * 2) != to.y / (VIEW_RANGE * 2))) {
		// ������� �����ϱ� ���� ���� �Ŵ� ������ ���Ѵ�.
		priority_queue<Short2> pq;
		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].lock();
		}
		// ���� ���Ϳ��� ������Ʈ�� ID�� �����Ѵ�.
		g_sector[from.y / (VIEW_RANGE * 2)][from.x / (VIEW_RANGE * 2)].erase(id);
		// ���ο� ���Ϳ� ������Ʈ�� ID�� �����Ѵ�.
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
		// �ֺ� 9���� ���� ���� ����
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

	// �� View List�� �߰��Ǿ��µ� ������ ���� �÷��̾��� ���� �߰�
	for (auto& cid : newViewList) {
		auto client = static_pointer_cast<CLIENT>(m_objects[cid]);
		client->m_viewLock.lock();
		if (!client->m_viewList.count(id)) {
			// View List�� ���ٸ� �����ش�.
			client->m_viewLock.unlock();
			client->SendAddPlayer(id);
		}
		else {
			// �ִٸ� �̵� ��Ų��.
			client->m_viewLock.unlock();
			client->SendMoveObject(id);
		}
	}

	// �� View List���� ���ŵǾ��µ� ������ �ִ� �÷��̾��� ���� ����
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
		// �̵� ���� ó��
		cout << "�̵� �Ұ���" << endl;
		return;
	}

	unordered_set<int> viewList;
	{
		short sectorX = monster->m_position.x / (VIEW_RANGE * 2);
		short sectorY = monster->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// �ֺ� 9���� ���� ���� ����
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
				// ���� ���� ������Ʈ �� �þ� ���� ������Ʈ�� �� ����Ʈ�� �߰�
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
	}

	// �̵� ó��
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

	// ���� ������ ��ġ�� �ٲ���� ���
	if ((from.x / (VIEW_RANGE * 2) != to.x / (VIEW_RANGE * 2)) ||
		(from.y / (VIEW_RANGE * 2) != to.y / (VIEW_RANGE * 2))) {
		// ������� �����ϱ� ���� ���� �Ŵ� ������ ���Ѵ�.
		priority_queue<Short2> pq;
		pq.push(from); pq.push(to);
		while (!pq.empty()) {
			auto index = pq.top(); pq.pop();
			g_sectorLock[index.y / (VIEW_RANGE * 2)][index.x / (VIEW_RANGE * 2)].lock();
		}
		// ���� ���Ϳ��� ������Ʈ�� ID�� �����Ѵ�.
		g_sector[from.y / (VIEW_RANGE * 2)][from.x / (VIEW_RANGE * 2)].erase(id);
		// ���ο� ���Ϳ� ������Ʈ�� ID�� �����Ѵ�.
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
		// �ֺ� 9���� ���� ���� ����
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

	// �� View List�� �߰��Ǿ��µ� ������ ���� �÷��̾��� ���� �߰�
	for (auto& cid : newViewList) {
		auto client = static_pointer_cast<CLIENT>(m_objects[cid]);
		client->m_viewLock.lock();
		if (!client->m_viewList.count(id)) {
			// View List�� ���ٸ� �����ش�.
			client->m_viewLock.unlock();
			client->SendAddPlayer(id);
		}
		else {
			// �ִٸ� �̵� ��Ų��.
			client->m_viewLock.unlock();
			client->SendMoveObject(id);
		}
	}

	// �� View List���� ���ŵǾ��µ� ������ �ִ� �÷��̾��� ���� ����
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

	// AStar ����
	// pair.first = �޸���ƽ
	// pair.second = �ش� ��ǥ
	priority_queue<pair<int, Short2>, vector<pair<int, Short2>>, greater<pair<int, Short2>>> pq;
	unordered_map<Short2, int> open;
	unordered_map<Short2, Short2> parent;
	// �̹� Ž���� ���(���� ���)�� ���
	unordered_set<Short2> closed;

	// ��� ��忡�� ���� �������� ���� ����� ���Ѵ�.
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

		// ���� 4ĭ�� ��ǥ�� �����Ѵ�.
		const array<Short2, 4> d = { Short2{0, 1},Short2{0, -1},Short2{1, 0} ,Short2{-1, 0} };
		for (int i = 0; i < 4; ++i) {
			const Short2 to = { top.second + d[i]};
			// �ش� ĭ���� �̵� �Ұ����ϸ� ť�� �߰� X
			if (m_map[to.y][to.x] & TileInfo::BLOCKING) {
				continue;
			}
			// �ش� ��带 �̹� �湮������ ť�� �߰� X
			if (closed.find(to) != closed.end()) {
				continue;
			}
			if (to.x > W_WIDTH || to.x < 0 || to.y > W_HEIGHT || to.y < 0) continue;

			// ������ Ž�� ����
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

			// �޸���ƽ : ��ֹ��� ���ٰ� ������ ������������ �Ÿ�
			int h = abs(to.x - end.x) + abs(to.y - end.y);
			// g : �ش� ���� �̵��ϱ� ���� ���
			int g = top.first + 1;
			int f = h + g;

			// ���� ���� ��忡 �ش� ��尡 ������ ����� �� ���� ��쿡�� ����
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

	// ��ǥ Ž�� ����
	lua_pushnumber(state, -1);
	return 1;
}


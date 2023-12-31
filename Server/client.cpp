#include "client.h"

CLIENT::CLIENT() : OBJECT(), m_socket{0}, m_isStress{false}
{}

CLIENT::~CLIENT() { closesocket(m_socket); }

void CLIENT::AutoHeal()
{
	if (m_hp >= m_maxHp) return;
	if (m_state != OBJECT::LIVE) return;

	m_hp += m_maxHp / 10;
	if (m_hp > m_maxHp) m_hp = m_maxHp;
	SendStatChange();
}

void CLIENT::Attacked(UINT attacker)
{
	if (m_state != OBJECT::LIVE) {
		return;
	}
	auto damage = g_gameServer.GetNPC(attacker)->GetAttackDamage();

	m_hp -= damage;

	if (m_hp <= 0) {
		Dead(attacker);
		return;
	}
	SendStatChange();

	// 시야 내 플레이어에게 체력이 바뀌었음을 알린다.
	unordered_set<int> playerList;
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
	// 주변 9개의 섹터 전부 조사
	for (int i = 0; i < 9; ++i) {
		if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
			sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
			continue;
		}

		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
		for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
			if (id >= MAX_USER) continue;
			auto& client = g_gameServer.GetClient(id);
			if (!(client->m_state & OBJECT::INGAME)) continue;
			if (CanSee(client->m_position)) {
				playerList.insert(id);
			}
		}
		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
	}

	for (auto player : playerList) {
		SC_CHANGE_HP_PACKET packet;
		packet.size = sizeof(SC_CHANGE_HP_PACKET);
		packet.type = SC_CHANGE_HP;
		packet.id = m_id;
		packet.hp = m_hp;
		packet.max_hp = m_maxHp;
		g_gameServer.GetClient(player)->DoSend(&packet);
	}
}

void CLIENT::Dead(UINT attacker)
{
	// 죽어서 LIVE -> DEAD 상태가 되면 시야 처리시 생략한다.
	// 따라서 섹터에서 제외해줄 필요는 없다.
	{
		unique_lock<mutex> lock(m_mutex);
		m_state = OBJECT::DEAD;
	}
	m_hp = m_maxHp;
	m_exp /= 2;
	SendStatChange();

	Timer::GetInstance().AddTimerEvent(m_id, TimerEvent::RESURRECTION,
		chrono::system_clock::now() + 10s, 0, -1);

	// 시야 내 플레이어에게 죽었음을 알린다.
	unordered_set<int> playerList;
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
	// 주변 9개의 섹터 전부 조사
	for (int i = 0; i < 9; ++i) {
		if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
			sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
			continue;
		}

		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
		for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
			if (id >= MAX_USER) continue;
			auto& client = g_gameServer.GetClient(id);
			if (!(client->m_state & OBJECT::INGAME)) continue;
			if (CanSee(client->m_position)) {
				playerList.insert(id);
			}
		}
		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
	}

	for (auto player : playerList) {
		SC_DEAD_OBJECT_PACKET packet;
		packet.size = sizeof(SC_DEAD_OBJECT_PACKET);
		packet.type = SC_DEAD_OBJECT;
		packet.id = m_id;
		g_gameServer.GetClient(player)->DoSend(&packet);
#ifdef NETWORK_DEBUG
		cout << "SC_DEAD_OBJECT 송신" << endl;
#endif
	}
}

void CLIENT::ExpUp(INT exp)
{
	m_exp += exp;

	// 만렙 : 20
	auto levelupExp = (int)pow(2, m_level - 1) * 100;
	if (m_exp >= levelupExp) {
		if (m_level == 20) {
			m_exp = levelupExp;
		}
		else {
			m_exp -= levelupExp;
			++m_level;

		}
	}
	SendStatChange();
}


void CLIENT::DoRecv()
{
	DWORD recv_flag = 0;
	memset(&m_recvOver.m_overlapped, 0, sizeof(m_recvOver.m_overlapped));
	m_recvOver.m_wsaBuf.len = BUF_SIZE - m_prevRemain;

	m_recvOver.m_wsaBuf.buf = m_recvOver.m_sendMsg + m_prevRemain;
	int retval = WSARecv(m_socket, &m_recvOver.m_wsaBuf, 1, 0, &recv_flag, 
		&m_recvOver.m_overlapped, 0);
}

void CLIENT::DoSend(void* packet)
{
	EXPOVERLAPPED* send_over = new EXPOVERLAPPED(reinterpret_cast<char*>(packet));
	WSASend(m_socket, &send_over->m_wsaBuf, 1, 0, 0, &send_over->m_overlapped, 0);
}

void CLIENT::SendLoginConfirm()
{
}

void CLIENT::SendAddPlayer(INT id)
{
	// id를 가진 플레이어를 새로 생성한다.
	m_viewLock.lock();
	if (m_viewList.count(id)) {
		// 해당 id를 가진 플레이어가 시야에 이미 존재한다.
		m_viewLock.unlock();
		// 이동만 시키고 종료한다.
		SendMoveObject(id);
		return;
	}
	m_viewList.insert(id);
	m_viewLock.unlock();

	SC_ADD_OBJECT_PACKET sendpk;
	sendpk.size = sizeof(SC_ADD_OBJECT_PACKET);
	sendpk.type = SC_ADD_OBJECT;
	sendpk.id = id;
	if (id < MAX_USER) {
		auto& client = g_gameServer.GetClient((UINT)id);
		sendpk.serial = client->m_serial;
		sendpk.coord = client->m_position;
		strcpy_s(sendpk.name, client->m_name);
		sendpk.level = client->m_level;
		sendpk.hp = client->m_hp;
		sendpk.maxHp = client->m_maxHp;
	}
	else {
		auto& npc = g_gameServer.GetNPC((UINT)id);
		sendpk.coord = npc->m_position;
		sendpk.serial = npc->m_serial;
		strcpy_s(sendpk.name, npc->m_name);
		sendpk.level = npc->m_level;
		sendpk.hp = npc->m_hp;
		sendpk.maxHp = npc->m_maxHp;
	}

	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_ADD_OBJECT 송신 - ID : " << m_id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendMoveObject(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// 시야에 해당 오브젝트가 존재하지 않는다면
		m_viewLock.unlock();
		// 그 오브젝트를 먼저 추가해준다.
		SendAddPlayer(id);
		return;
	}
	m_viewLock.unlock();

	SC_MOVE_OBJECT_PACKET sendpk;
	sendpk.size = sizeof(SC_MOVE_OBJECT_PACKET);
	sendpk.type = SC_MOVE_OBJECT;
	sendpk.id = id;
	if (id < MAX_USER) {
		sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
		sendpk.move_time = g_gameServer.GetClient(id)->m_lastMoveTime;
	}
	else {
		sendpk.coord = g_gameServer.GetNPC((UINT)id)->m_position;
		sendpk.move_time = 0;
	}

	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_MOVE_OBJECT 송신 - ID : " << m_id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendChat(const char* message)
{
	// 시야 내 플레이어에게 채팅 전송
	unordered_set<int> playerList;
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
	// 주변 9개의 섹터 전부 조사
	for (int i = 0; i < 9; ++i) {
		if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
			sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
			continue;
		}

		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
		for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
			if (id >= MAX_USER) continue;
			auto& client = g_gameServer.GetClient(id);
			if (!(client->m_state & OBJECT::INGAME)) continue;
			if (CanSee(client->m_position)) {
				playerList.insert(id);
			}
		}
		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
	}

	string sendstr = message;
	SC_CHAT_PACKET packet;
	packet.size = sizeof(PACKET) + sizeof(packet.id) + sendstr.size() + 1;
	packet.type = SC_CHAT;
	packet.id = m_id;
	strcpy_s(packet.message, sendstr.c_str());

	for (auto player : playerList) {
		g_gameServer.GetClient(player)->DoSend(&packet);
#ifdef NETWORK_DEBUG
		cout << "SC_CHAT 송신 - ID : " << m_id;
#endif
	}
}

void CLIENT::SendExitPlayer(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// 이미 시야에 해당 오브젝트가 존재하지 않는다면
		m_viewLock.unlock();
		// 아무 일도 하지 않는다.
		return;
	}
	m_viewList.erase(id);
	m_viewLock.unlock();

	SC_REMOVE_OBJECT_PACKET packet;
	packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	packet.type = SC_REMOVE_OBJECT;
	packet.id = id;
	DoSend(&packet);

#ifdef NETWORK_DEBUG
	cout << "SC_REMOVE_OBJECT 송신 - ID : " << m_id << endl;
#endif
}

void CLIENT::SendStatChange()
{
	SC_STAT_CHANGE_PACKET packet;
	packet.size = sizeof(SC_STAT_CHANGE_PACKET);
	packet.type = SC_STAT_CHANGE;
	packet.hp = m_hp;
	packet.max_hp = m_maxHp;
	packet.exp = m_exp;
	packet.level = m_level;
	DoSend(&packet);

#ifdef NETWORK_DEBUG
	cout << "SC_STAT_CHANGE 송신 - ID : " << m_id << endl;
#endif
}

void CLIENT::SendAddEffect(Short2 position)
{
	// 시야 내 플레이어에게 효과 전송
	unordered_set<int> playerList;
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
	// 주변 9개의 섹터 전부 조사
	for (int i = 0; i < 9; ++i) {
		if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
			sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
			continue;
		}

		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
		for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
			if (id >= MAX_USER) continue;
			auto& client = g_gameServer.GetClient(id);
			if (!(client->m_state & OBJECT::INGAME)) continue;
			if (CanSee(client->m_position)) {
				playerList.insert(id);
			}
		}
		g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
	}

	SC_ADD_EFFECT_PACKET packet;
	packet.size = sizeof(SC_ADD_EFFECT_PACKET);
	packet.type = SC_ADD_EFFECT;
	if (m_serial == Serial::Character::HAKUREI_REIMU) {
		packet.serial = Serial::Effect::REIMU_SKILL;
	}
	if (m_serial == Serial::Character::PATCHOULI_KNOWLEDGE) {
		packet.serial = Serial::Effect::PATCHOULI_SKILL;
	}
	packet.coord = position;
	for (auto player : playerList) {
		g_gameServer.GetClient(player)->DoSend(&packet);
#ifdef NETWORK_DEBUG
		cout << "SC_ADD_EFFECT 송신 - ID : " << m_id;
#endif
	}
}

int CLIENT::GetAttackDamage()
{
	return m_baseAtk + m_bonusAtk * m_level;
}

int CLIENT::GetSkillDamage()
{
	return m_baseSkill + m_bonusSkill * m_level;
}
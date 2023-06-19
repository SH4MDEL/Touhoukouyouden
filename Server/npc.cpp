#include "npc.h"
#include "server.h"

NPC::NPC() : OBJECT(), m_isActive{false}, m_attacked{-1}
{
}

void NPC::Attacked(UINT attacker)
{
	auto damage = g_gameServer.GetClient(attacker)->GetAttackDamage();
	if (m_attacked == -1) m_attacked = attacker;

	m_hp -= damage;

	if (m_hp <= 0) {
		Dead(attacker);
		return;
	}

	unordered_set<int> playerList;
	// 시야 내 플레이어에게 체력이 바뀌었음을 알린다.
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	g_sectorLock[sectorY][sectorX].lock();
	for (auto& cid : g_sector[sectorY][sectorX]) {
		if (cid >= MAX_USER) continue;
		auto& client = g_gameServer.GetClient(cid);
		if (!(client->m_state & OBJECT::INGAME)) continue;
		if (CanSee(client->m_position)) {
			playerList.insert(cid);
		}
	}
	g_sectorLock[sectorY][sectorX].unlock();

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

void NPC::Dead(UINT attacker)
{
	// 죽어서 LIVE -> DEAD 상태가 되면 시야 처리시 생략한다.
	// 따라서 섹터에서 제외해줄 필요는 없다.
	{
		unique_lock<mutex> lock(m_mutex);
		m_state = OBJECT::DEAD;
	}
	m_hp = m_maxHp;
	m_attacked = -1;

	Timer::GetInstance().AddTimerEvent(m_id, TimerEvent::RESURRECTION,
		chrono::system_clock::now() + 10s, 0, -1);

	// 먼저 나를 죽인 플레이어의 경험치를 올린다.
	g_gameServer.GetClient(attacker)->ExpUp(m_exp);

	// 시야 내 플레이어에게 죽었음을 알린다.
	unordered_set<int> playerList;
	short sectorX = m_position.x / (VIEW_RANGE * 2);
	short sectorY = m_position.y / (VIEW_RANGE * 2);
	g_sectorLock[sectorY][sectorX].lock();
	for (auto& cid : g_sector[sectorY][sectorX]) {
		if (cid >= MAX_USER) continue;
		auto& client = g_gameServer.GetClient(cid);
		if (!(client->m_state & OBJECT::INGAME)) continue;
		if (CanSee(client->m_position)) {
			playerList.insert(cid);
		}
	}
	g_sectorLock[sectorY][sectorX].unlock();

	for (auto player : playerList) {
		SC_DEAD_OBJECT_PACKET packet;
		packet.size = sizeof(SC_DEAD_OBJECT_PACKET);
		packet.type = SC_DEAD_OBJECT;
		packet.id = m_id;
		g_gameServer.GetClient(player)->DoSend(&packet);
	}
}

int NPC::GetAttackDamage()
{
	return m_atk;
}

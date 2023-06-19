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
	// �þ� �� �÷��̾�� ü���� �ٲ������ �˸���.
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
	// �׾ LIVE -> DEAD ���°� �Ǹ� �þ� ó���� �����Ѵ�.
	// ���� ���Ϳ��� �������� �ʿ�� ����.
	{
		unique_lock<mutex> lock(m_mutex);
		m_state = OBJECT::DEAD;
	}
	m_hp = m_maxHp;
	m_attacked = -1;

	Timer::GetInstance().AddTimerEvent(m_id, TimerEvent::RESURRECTION,
		chrono::system_clock::now() + 10s, 0, -1);

	// ���� ���� ���� �÷��̾��� ����ġ�� �ø���.
	g_gameServer.GetClient(attacker)->ExpUp(m_exp);

	// �þ� �� �÷��̾�� �׾����� �˸���.
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

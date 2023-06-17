#include "npc.h"

NPC::NPC() : OBJECT(), m_isActive{false}
{
}

void NPC::Attacked(UINT attacker)
{
	cout << attacker << "로부터 공격" << endl;
}

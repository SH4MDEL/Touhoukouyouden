#pragma once
#include "object.h"

class NPC : public OBJECT
{
public:
	NPC();
	~NPC() override = default;

	void Attacked(UINT attacker) override;
	void Skilled(UINT attacker);
	void Dead(UINT attacker) override;

	int GetAttackDamage();

public:
	atomic_bool	m_isActive;
	atomic_int	m_attacked;

	lua_State*	m_monsterState;
	mutex		m_luaLock;

	chrono::milliseconds m_speed;
	INT			m_atk;
	INT			m_moveType;
	INT			m_waitType;
};


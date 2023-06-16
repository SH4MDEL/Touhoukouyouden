#pragma once
#include "object.h"

class NPC : public OBJECT
{
public:
	NPC();
	~NPC() override = default;

public:
	atomic_bool	m_isActive;

	lua_State*	m_luaState;
	mutex		m_luaLock;

	FLOAT		m_speed;
	INT			m_type;
};


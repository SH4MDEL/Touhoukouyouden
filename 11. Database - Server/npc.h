#pragma once
#include "object.h"

class NPC : public OBJECT
{
public:
	NPC();
	~NPC() override = default;

public:
	atomic_bool	m_isActive;
	atomic_bool m_luaInit;

	lua_State*	m_luaState;
	mutex		m_luaLock;
};


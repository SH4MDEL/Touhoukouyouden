#pragma once
#include "stdafx.h"

class OBJECT
{
public:
	enum State { FREE, ALLOC, INGAME };
public:
	OBJECT();
	virtual ~OBJECT() = default;

	virtual void DoRecv() {};
	virtual void DoSend(void* packet) {};

public:
	mutex				m_mutex;
	State				m_state;
	INT					m_id;

	Short2				m_position;
	char				m_name[NAME_SIZE];

	INT					m_prevRemain;
	INT					m_lastMoveTime;
};

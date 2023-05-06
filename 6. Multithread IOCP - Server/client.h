#pragma once
#include "stdafx.h"
#include "exp_over.h"
#include "main.h"

class CLIENT
{
public:
	enum State { FREE, ALLOC, INGAME };
public:
	CLIENT();
	~CLIENT();

	void DoRecv();
	void DoSend(void* packet);

public:
	EXP_OVER	m_recvOver;
	mutex		m_mutex;
	State		m_state;
	INT			m_id;
	SOCKET		m_socket;

	Short2		m_position;
	char		m_name[NAMESIZE];

	INT			m_prevRemain;
};


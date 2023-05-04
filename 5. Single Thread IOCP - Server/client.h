#pragma once
#include "stdafx.h"
#include "exp_over.h"
#include "main.h"

class CLIENT
{
public:
	CLIENT();
	CLIENT(UINT id, SOCKET socket, Short2 position);
	~CLIENT();

	void DoRecv();
	void DoSend(void* packet);

public:
	EXP_OVER	m_recvOver;
	UINT		m_id;

	SOCKET				m_socket;
	Short2		m_position;
	char				m_name[NAMESIZE];

	INT					m_prevRemain;
};


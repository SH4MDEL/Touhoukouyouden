#pragma once
#include "stdafx.h"
#include "object.h"
#include "exp_over.h"
#include "main.h"

class CLIENT : public OBJECT
{
public:
	CLIENT();
	~CLIENT() override;

	void DoRecv() override;
	void DoSend(void* packet) override;

	void SendLoginConfirm();
	void SendAddPlayer(INT id);
	void SendObjectInfo(INT id);
	void SendChat(INT id, const char* message);
	void SendExitPlayer(INT id);

public:
	EXP_OVER			m_recvOver;
	SOCKET				m_socket;

	unordered_set<int>	m_viewList;
	mutex				m_viewLock;
};


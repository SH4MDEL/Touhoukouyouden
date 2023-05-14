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

	void SendLoginConfirm() override;
	void SendAddPlayer(INT id) override;
	void SendObjectInfo(INT id) override;
	void SendExitPlayer(INT id) override;

public:
	EXP_OVER			m_recvOver;
	SOCKET				m_socket;

	unordered_set<int>	m_viewList;
	mutex				m_viewLock;
};


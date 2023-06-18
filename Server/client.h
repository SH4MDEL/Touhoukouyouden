#pragma once
#include "stdafx.h"
#include "object.h"
#include "expOverlapped.h"
#include "main.h"

class CLIENT : public OBJECT
{
public:
	CLIENT();
	~CLIENT() override;

	void AutoHeal();
	void Attacked(UINT attacker) override;
	void Dead(UINT attacker) override;
	void ExpUp(INT exp);

	void DoRecv() override;
	void DoSend(void* packet) override;

	void SendLoginConfirm();
	void SendAddPlayer(INT id);
	void SendMoveObject(INT id);
	void SendChat(INT id, const char* message);
	void SendExitPlayer(INT id);
	void SendStatChange();

	int GetAttackDamage();
	int GetSkillDamage();

public:
	EXPOVERLAPPED		m_recvOver;
	SOCKET				m_socket;

	unordered_set<int>	m_viewList;
	mutex				m_viewLock;

	int m_baseAtk;
	int m_bonusAtk;
	int m_baseSkill;
	int m_bonusSkill;
};


#pragma once
#include "stdafx.h"
#include "npc.h"
#include "client.h"

struct Event
{
	enum Type { RANDOM_MOVE, HEAL, ATTACK };

	UINT m_id;
	Type m_type;
	chrono::system_clock::time_point m_executeTime;

	constexpr bool operator<(const Event& rhs) const {
		return m_executeTime > rhs.m_executeTime;
	}
};

class OBJECT;
class CLIENT;
class NPC;
class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	void InitializeNPC();

	UINT RegistClient(const SOCKET& c_socket);
	void ExitClient(UINT id);

	Short2 GetPlayerPosition(UINT id);
	BOOL CanSee(UINT id1, UINT id2);
	void Move(UINT id, UCHAR direction);

	shared_ptr<CLIENT> GetClient(UINT id);
	shared_ptr<NPC> GetNPC(UINT id);

	void AddTimer(UINT id, Event::Type type, chrono::system_clock::time_point executeTime);
	void WakeupNPC(UINT id);

	void MoveNPC(UINT id);

	void TimerThread(HANDLE hiocp);

private:
	array<shared_ptr<OBJECT>, MAX_USER + MAX_NPC> m_objects;

	concurrency::concurrent_priority_queue<Event> m_timerQueue;
};



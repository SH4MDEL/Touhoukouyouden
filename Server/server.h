#pragma once
#include "stdafx.h"
#include "npc.h"
#include "client.h"

struct TimerEvent
{
	enum Type { RANDOM_MOVE, HEAL, ATTACK };

	UINT m_id;
	Type m_type;
	chrono::system_clock::time_point m_executeTime;
	INT m_eventMsg;
	UINT m_targetid;

	constexpr bool operator<(const TimerEvent& rhs) const {
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

	void LoadMap();
	void InitializeNPC();

	UINT RegistClient(const SOCKET& c_socket);
	void RegistClientPosition(UINT id, Short2 position);
	void ExitClient(UINT id);

	Short2 GetPlayerPosition(UINT id);
	BOOL CanSee(UINT id1, UINT id2);
	BOOL IsSamePosition(UINT id1, UINT id2);
	void Move(UINT id, UCHAR direction);

	shared_ptr<CLIENT> GetClient(UINT id);
	shared_ptr<NPC> GetNPC(UINT id);

	void AddTimerEvent(UINT id, TimerEvent::Type type, chrono::system_clock::time_point executeTime, INT eventMsg, UINT targetid);
	void WakeupNPC(UINT id, UINT waker);
	void SleepNPC(UINT id);
	void MoveNPC(UINT id);

	void TimerThread(HANDLE hiocp);

	// Lua
	INT Lua_GetX(lua_State* state);
	INT Lua_GetY(lua_State* state);
	int Lua_SendMessage(lua_State* state);

private:
	array<array<int, W_WIDTH>, W_HEIGHT>			m_map;
	array<shared_ptr<OBJECT>, MAX_USER + MAX_NPC>	m_objects;

	concurrency::concurrent_priority_queue<TimerEvent> m_timerQueue;
};



#pragma once
#include "stdafx.h"
#include "npc.h"
#include "client.h"

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
	BOOL IsSamePosition(UINT id1, UINT id2);
	void Move(UINT id, UCHAR direction);

	shared_ptr<CLIENT> GetClient(UINT id);
	shared_ptr<NPC> GetNPC(UINT id);

	void AddTimer(UINT id, Event::Type type, chrono::system_clock::time_point executeTime, INT eventMsg, UINT targetid);
	void WakeupNPC(UINT id, UINT waker);
	void SleepNPC(UINT id);
	void MoveNPC(UINT id);

	void TimerThread(HANDLE hiocp);

	// Lua
	INT Lua_GetX(lua_State* state);
	INT Lua_GetY(lua_State* state);
	int Lua_SendMessage(lua_State* state);

private:
	array<shared_ptr<OBJECT>, MAX_USER + MAX_NPC> m_objects;
};



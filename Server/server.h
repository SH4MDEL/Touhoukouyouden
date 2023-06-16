#pragma once
#include "stdafx.h"
#include "npc.h"
#include "client.h"
#include "timer.h"

class OBJECT;
class CLIENT;
class NPC;
class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	void LoadMap();
	void InitializeMonster();

	UINT RegistClient(const SOCKET& c_socket);
	void InputClient(UINT id, int serial, Short2 position, int level, int exp, int hp, int maxHp);
	void ExitClient(UINT id);

	Short2 GetPlayerPosition(UINT id);
	BOOL CanSee(UINT id1, UINT id2);
	BOOL IsSamePosition(UINT id1, UINT id2);
	void Move(UINT id, UCHAR direction);

	shared_ptr<CLIENT> GetClient(UINT id);
	shared_ptr<NPC> GetNPC(UINT id);

	void WakeupNPC(UINT id, UINT waker);
	void SleepNPC(UINT id);
	void MoveNPC(UINT id);

	// Lua
	INT Lua_GetX(lua_State* state);
	INT Lua_GetY(lua_State* state);
	int Lua_SendMessage(lua_State* state);

private:
	array<array<int, W_WIDTH>, W_HEIGHT>			m_map;
	array<shared_ptr<OBJECT>, MAX_USER + MAX_MONSTER>	m_objects;
};



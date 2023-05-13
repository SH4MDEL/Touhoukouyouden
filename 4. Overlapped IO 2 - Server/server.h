#pragma once
#include "stdafx.h"
#include "session.h"

class SESSION;
class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	int RegistPlayer(UINT id);
	void ExitPlayer(UINT id);

	Short2 GetPlayerPosition(UINT id);
	Short2 Move(UINT id, UCHAR direction);
	Short2 GetPlayer(UINT id);
	unordered_map<UINT, Short2>& GetPlayers();

	SESSION& GetClient(UINT id);
	unordered_map<UINT, SESSION>& GetClients();
	void RegistClient(UINT id, const SOCKET& socket);
	void ExitClient(UINT id);
	void ResetClients();

private:
	unordered_map<UINT, Short2>	m_player;
	unordered_map<UINT, SESSION>	m_clients;
};


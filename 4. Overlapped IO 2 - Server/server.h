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
	POINT InputPlayer(UINT id);

	POINT GetPlayerPosition(UINT id);
	POINT Move(UINT id, UCHAR direction);
	POINT GetPlayer(UINT id);
	unordered_map<UINT, POINT>& GetPlayers();

	SESSION& GetClient(UINT id);
	unordered_map<UINT, SESSION>& GetClients();
	void RegistClient(UINT id, const SOCKET& socket);
	void ExitClient(UINT id);
	void ResetClients();

private:
	unordered_map<UINT, POINT>	m_player;
	unordered_map<UINT, SESSION>	m_clients;
};


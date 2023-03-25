#pragma once
#include "stdafx.h"
#include "session.h"

class SESSION;
class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	int RegistPlayer(int id);
	void ExitPlayer(int id);
	POINT InputPlayer(int id);

	POINT GetPlayerPosition(int id);
	POINT Move(int id, POINT d);
	POINT GetPlayer(int id);
	unordered_map<unsigned long long, POINT>& GetPlayers();

	SESSION& GetClient(unsigned long long id);
	unordered_map<unsigned long long, SESSION>& GetClients();
	void RegistClient(unsigned long long id, const SOCKET& socket);
	void ExitClient(int id);
	void ResetClients();

private:
	int							m_map[MAP_WIDTH][MAP_HEIGHT];
	unordered_map<unsigned long long, POINT>	m_player;
	unordered_map<unsigned long long, SESSION>	m_clients;
};


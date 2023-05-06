#pragma once
#include "stdafx.h"
#include "client.h"

class CLIENT;
class GameServer
{
public:
	GameServer();
	~GameServer() = default;

	UINT RegistClient(const SOCKET& c_socket);
	void ExitClient(UINT id);
	void ResetClients();

	Short2 GetPlayerPosition(UINT id);
	Short2 Move(UINT id, UCHAR direction);

	CLIENT& GetClient(UINT id);
	unordered_map<UINT, CLIENT>& GetClients();


private:
	unordered_map<UINT, CLIENT>	m_clients;
};


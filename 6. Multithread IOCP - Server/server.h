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

	Short2 GetPlayerPosition(UINT id);
	Short2 Move(UINT id, UCHAR direction);

	shared_ptr<CLIENT> GetClient(UINT id);
	array<shared_ptr<CLIENT>, MAX_USER>& GetClients();

private:
	array<shared_ptr<CLIENT>, MAX_USER> m_clients;
};


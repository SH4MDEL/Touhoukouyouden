#include "main.h"

int main()
{
	// �����ͺ��̽��� ����
	Database::GetInstance();
	Setting::GetInstance();
	g_gameServer.LoadMap();
	// NPC �ʱ�ȭ
	g_gameServer.InitializeMonster();

	// ���� ����
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(g_serverSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_serverSocket, SOMAXCONN);

	INT addr_size = sizeof(SOCKADDR_IN);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_serverSocket), g_iocp, 9999, 0);

	g_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	g_expOverlapped.m_compType = COMP_TYPE::OP_ACCEPT;
	AcceptEx(g_serverSocket, g_clientSocket, g_expOverlapped.m_sendMsg, 0, 
		addr_size + 16, addr_size + 16, 0, &g_expOverlapped.m_overlapped);

	thread timerThread{ TimerThread, g_iocp };
	thread dbThread{ DatabaseThread, g_iocp };

	vector<thread> workerThreads;
	auto numThreads = thread::hardware_concurrency();
	for (UINT i = 0; i < numThreads; ++i) {
		workerThreads.emplace_back(WorkerThread, g_iocp);
	}

	for (auto& thread : workerThreads) {
		thread.join();
	}
	timerThread.join();
	dbThread.join();

	closesocket(g_serverSocket);
	WSACleanup();
}

void WorkerThread(HANDLE hiocp)
{
	while (true) {
		DWORD received;
		ULONG_PTR key;
		WSAOVERLAPPED* overlapped = nullptr;

		BOOL ret = GetQueuedCompletionStatus(hiocp, &received, &key, &overlapped, INFINITE);
		EXPOVERLAPPED* expOverlapped = reinterpret_cast<EXPOVERLAPPED*>(overlapped);

		if (!ret) {
			if (expOverlapped->m_compType == COMP_TYPE::OP_ACCEPT) {
				cout << "OP_ACCEPT Error\n";
				exit(-1);
			}
			else {
				cout << "GetQueuedCompletionStatus Error on client[" << key << "]\n";
				// ���� ���� ��Ŷ ����
				g_gameServer.ExitClient(key);
				if (expOverlapped->m_compType == OP_SEND) delete expOverlapped;
				continue;
			}
		}

		if ((received == 0) && ((expOverlapped->m_compType == OP_RECV) || (expOverlapped->m_compType == OP_SEND))) {
			// ���� ���� ��Ŷ ����
			g_gameServer.ExitClient(key);
			if (expOverlapped->m_compType == OP_SEND) delete expOverlapped;
			continue;
		}

		switch (expOverlapped->m_compType)
		{
		case OP_ACCEPT:
		{
			UINT clientID = g_gameServer.RegistClient(g_clientSocket);
			if (clientID != -1) {
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_clientSocket),
					hiocp, clientID, 0);
				g_gameServer.GetClient(clientID)->DoRecv();
#ifdef NETWORK_DEBUG
				cout << "�÷��̾� ���� ����. ID : " << clientID << endl;
#endif
			}
			else {
				cout << "MAX USER EXCEEDED.\n";
				closesocket(g_clientSocket);
			}
			g_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&g_expOverlapped.m_overlapped, sizeof(g_expOverlapped.m_overlapped));
			INT addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_serverSocket, g_clientSocket, g_expOverlapped.m_sendMsg, 0,
				addr_size + 16, addr_size + 16, 0, &g_expOverlapped.m_overlapped);
			break;
		}
		case OP_RECV:
		{
			// ��Ŷ ������ �ʿ�
			size_t remainPacketSize = g_gameServer.GetClient(key)->m_prevRemain + received;
			char* remainPacketBuffer = expOverlapped->m_sendMsg;

			while (remainPacketSize > 0) {
				// ���� ������ ����� 0���� Ŭ ��
				int packetSize = remainPacketBuffer[0];

				if (remainPacketSize < packetSize) {
					g_gameServer.GetClient(key)->m_prevRemain = remainPacketSize;
					if (remainPacketSize != 0) {
						memcpy(expOverlapped->m_sendMsg, remainPacketBuffer, remainPacketSize);
					}
					break;
				}
				// �ϳ��� ������ ��Ŷ�� ����� ���� �������
				// ���ڶ� �� Ż��

				ProcessPacket(key, remainPacketBuffer);
				// ��Ŷ ó��

				remainPacketBuffer += packetSize;
				remainPacketSize -= packetSize;
				// ó���� ��ŭ ������ ��ġ�� �ڷ� ����
				// ������ ũ��� ����

				if (remainPacketSize != 0) {
					memcpy(expOverlapped->m_sendMsg, remainPacketBuffer, remainPacketSize);
				}
			}
			if (remainPacketSize == 0) g_gameServer.GetClient(key)->m_prevRemain = 0;
			g_gameServer.GetClient(key)->DoRecv();

			break;
		}
		case OP_SEND:
		{
			delete expOverlapped;
			break;
		}
		case TIMER_NPC_MOVE:
		{
			bool activate = false;
			auto& monster = g_gameServer.GetNPC(key);
			// ���Ͱ� AGRO Ÿ���̶�� �ϴ� ��� �߰�.
			UINT* targetID = reinterpret_cast<UINT*>(expOverlapped->m_sendMsg);
			if (monster->m_moveType == Type::Move::AGRO) {
				g_gameServer.PathfindingNPC(key, *targetID);
			}
			else if (monster->m_waitType == Type::Wait::ROAMING) {
				g_gameServer.RoamingNPC(key);
			}

			short sectorX = monster->m_position.x / (VIEW_RANGE * 2);
			short sectorY = monster->m_position.y / (VIEW_RANGE * 2);
			const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
			const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
			// �ֺ� 9���� ���� ���� ����
			for (int i = 0; i < 9; ++i) {
				if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
					sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
					continue;
				}

				g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
				for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
					if (id < MAX_USER) {
						if (g_gameServer.GetClient(id)->m_state != OBJECT::INGAME) continue;
						if (!g_gameServer.CanSee(key, id)) continue;
						activate = true;
						break;
					}
				}
				g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();

			}
			if (activate) {
				Timer::GetInstance().AddTimerEvent(key, TimerEvent::MOVE,
					chrono::system_clock::now() + monster->m_speed, 0, *targetID);
			}
			else {
				monster->m_isActive = false;
			}
			delete expOverlapped;
			break;
		}
		case TIMER_NPC_ATTACK:
		{
			int* cid = reinterpret_cast<int*>(expOverlapped->m_sendMsg);
			auto npc = g_gameServer.GetNPC(key);

			npc->m_luaLock.lock();
			lua_getglobal(npc->m_luaState, "event_player_move");
			lua_pushnumber(npc->m_luaState, *cid);
			lua_pcall(npc->m_luaState, 1, 0, 0);
			npc->m_luaLock.unlock();

			delete expOverlapped;
			break;
		}

		case DB_LOGIN_OK:
		{
			UINT* cid = reinterpret_cast<UINT*>(expOverlapped->m_sendMsg);
			DataBaseUserInfo* userinfo = 
				reinterpret_cast<DataBaseUserInfo*>(expOverlapped->m_sendMsg + sizeof(UINT));

			auto& client = g_gameServer.GetClient(*cid);
			{
				SC_LOGIN_OK_PACKET sendpk;
				sendpk.size = sizeof(SC_LOGIN_OK_PACKET);
				sendpk.type = SC_LOGIN_OK;
				client->DoSend(&sendpk);
				strcpy_s(client->m_name, userinfo->id);
#ifdef NETWORK_DEBUG
				cout << "SC_LOGIN_OK �۽�" << endl;
#endif
				{
					unique_lock<mutex> lock(client->m_mutex);
					client->m_state = OBJECT::INGAME;
				}
			}
			{
				SC_LOGIN_INFO_PACKET sendpk;
				sendpk.size = sizeof(SC_LOGIN_INFO_PACKET);
				sendpk.type = SC_LOGIN_INFO;
				sendpk.id = *cid;
				sendpk.hp = client->m_hp;
				sendpk.max_hp = client->m_maxHp;
				sendpk.exp = client->m_exp;
				sendpk.level = client->m_level;
				client->DoSend(&sendpk);
#ifdef NETWORK_DEBUG
				cout << "SC_LOGIN_INFO �۽�" << endl;
#endif
			}

			// ���� �ȿ� �ִ� ������Ʈ�� ID�� ����.
			unordered_set<int> newViewList;
			short sectorX = client->m_position.x / (VIEW_RANGE * 2);
			short sectorY = client->m_position.y / (VIEW_RANGE * 2);
			const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
			const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
			// �ֺ� 9���� ���� ���� ����
			for (int i = 0; i < 9; ++i) {
				if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
					sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
					continue;
				}

				g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
				for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
					{
						if (id < MAX_USER) {
							unique_lock<mutex> lock(g_gameServer.GetClient(id)->m_mutex);
							if (g_gameServer.GetClient(id)->m_state != OBJECT::INGAME) continue;
						}
						else {
							unique_lock<mutex> lock(g_gameServer.GetNPC(id)->m_mutex);
							if (g_gameServer.GetNPC(id)->m_state != OBJECT::INGAME) continue;
						}
					}
					if (!g_gameServer.CanSee(*cid, id)) continue;

					// �ϴ� ������ ����
					client->SendAddPlayer(id);
					// ���� NPC�� �ƴ� ��� ����
					if (id < MAX_USER) g_gameServer.GetClient(id)->SendAddPlayer(*cid);
					else if (g_gameServer.IsSamePosition(id, *cid)) g_gameServer.WakeupNPC(id, *cid);
				}
				g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
			}
			delete expOverlapped;
			break;
		}
		case DB_LOGIN_FAIL:
		{
			UINT* cid = reinterpret_cast<UINT*>(expOverlapped->m_sendMsg);
			DataBaseUserInfo* userinfo =
				reinterpret_cast<DataBaseUserInfo*>(expOverlapped->m_sendMsg + sizeof(UINT));

			SC_LOGIN_FAIL_PACKET sendpk;
			sendpk.size = sizeof(SC_LOGIN_FAIL_PACKET);
			sendpk.type = SC_LOGIN_FAIL;
			g_gameServer.GetClient(*cid)->DoSend(&sendpk);
#ifdef NETWORK_DEBUG
			cout << "SC_LOGIN_FAIL �۽� - ID : " << cid << endl;
#endif
			delete expOverlapped;
			break;
		}
		}
	}
}

void ProcessPacket(UINT cid, CHAR* packetBuf)
{
	switch (packetBuf[2])
	{
	case CS_LOGIN:
	{
		// ���� �÷��̾� ����
		CS_LOGIN_PACKET* pk = reinterpret_cast<CS_LOGIN_PACKET*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_LOGIN ����" << endl;
#endif
		Database::GetInstance().AddDatabaseEvent(DatabaseEvent{
			(UINT)cid, DatabaseEvent::Type::LOGIN,
			DataBaseUserInfo { pk->id, pk->password, -1, -1 }
			});
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* pk = reinterpret_cast<CS_MOVE_PACKET*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_MOVE ����" << endl;
#endif

		g_gameServer.Move(cid, (*pk).direction);
		g_gameServer.GetClient(cid)->m_lastMoveTime = pk->moveTime;
		
		// ���� �ȿ� �ִ� ������Ʈ�� ID�� ����.
		unordered_set<int> newViewList;
		g_gameServer.GetClient(cid)->m_viewLock.lock();
		auto oldViewList = g_gameServer.GetClient(cid)->m_viewList;
		g_gameServer.GetClient(cid)->m_viewLock.unlock();

		short sectorX = g_gameServer.GetClient(cid)->m_position.x / (VIEW_RANGE * 2);
		short sectorY = g_gameServer.GetClient(cid)->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// �ֺ� 9���� ���� ���� ����
		for (int i = 0; i < 9; ++i) {
			if (sectorX + dx[i] >= W_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= W_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
				continue;
			}

			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].lock();
			for (auto& id : g_sector[sectorY + dy[i]][sectorX + dx[i]]) {
				if (id < MAX_USER) {
					if (g_gameServer.GetClient(id)->m_state != OBJECT::INGAME) continue;
				}
				else {
					if (g_gameServer.GetNPC(id)->m_state != OBJECT::INGAME) continue;
				}
				if (!g_gameServer.CanSee(cid, id)) continue;

				newViewList.insert(id);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}

		// �� View List�� �߰��Ǿ��µ� ������ ���� �÷��̾��� ���� �߰�
		for (auto& id : newViewList) {
			g_gameServer.GetClient(cid)->m_viewLock.lock();
			if (!g_gameServer.GetClient(cid)->m_viewList.count(id)) {
				// View List�� ���ٸ� �����ش�.
				g_gameServer.GetClient(cid)->m_viewLock.unlock();
				g_gameServer.GetClient(cid)->SendAddPlayer(id);
				if (id < MAX_USER) g_gameServer.GetClient(id)->SendAddPlayer(cid);
			}
			else {
				// �ִٸ� �̵� ��Ų��.
				g_gameServer.GetClient(cid)->m_viewLock.unlock();
				if (id < MAX_USER) g_gameServer.GetClient(id)->SendObjectInfo(cid);
			}

			if (!oldViewList.count(id)) {
				g_gameServer.GetClient(cid)->SendAddPlayer(id);
				if (id >= MAX_USER) g_gameServer.WakeupNPC(id, cid);
			}
		}
		
		// �� View List�� ���ŵǾ��µ� ������ �ִ� �÷��̾��� ���� ����
		for (auto& id : oldViewList) {
			if (!newViewList.count(id)) {
				g_gameServer.GetClient(cid)->SendExitPlayer(id);
				if (id < MAX_USER) g_gameServer.GetClient(id)->SendExitPlayer(cid);
			}
		}

		// �̵��� �ڸ��� NPC�� �ִ��� �˻�
		// ���� ��� �ǰ� ������ �ο�
		g_gameServer.GetClient(cid)->m_viewLock.lock();
		auto npcCheckingViewList = g_gameServer.GetClient(cid)->m_viewList;
		g_gameServer.GetClient(cid)->m_viewLock.unlock();
		for (auto& id : npcCheckingViewList) {
			if (id >= MAX_USER && g_gameServer.IsSamePosition(id, cid)) {
				g_gameServer.WakeupNPC(id, cid);
			}
		}

		break;
	}
	case CS_ATTACK:
	{
		CS_ATTACK_PACKET* pk = reinterpret_cast<CS_ATTACK_PACKET*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_ATTACK ����" << endl;
#endif
		g_gameServer.Attack(cid, pk->direction);
		break;
	}
	}
}

void TimerThread(HANDLE hiocp)
{
	Timer::GetInstance().TimerThread(hiocp);
}

void DatabaseThread(HANDLE hiocp)
{
	Database::GetInstance().DatabaseThread(hiocp);
}
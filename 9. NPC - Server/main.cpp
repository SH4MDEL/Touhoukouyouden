#include "main.h"

int main()
{
	HANDLE hiocp;

	g_gameServer.InitializeNPC();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(g_serverSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_serverSocket, SOMAXCONN);

	INT addr_size = sizeof(SOCKADDR_IN);

	hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_serverSocket), hiocp, 9999, 0);

	g_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	g_expOverlapped.m_compType = COMP_TYPE::OP_ACCEPT;
	AcceptEx(g_serverSocket, g_clientSocket, g_expOverlapped.m_sendMsg, 0, 
		addr_size + 16, addr_size + 16, 0, &g_expOverlapped.m_overlapped);

	thread timerThread{ TimerThread, hiocp };

	vector<thread> workerThreads;
	auto numThreads = thread::hardware_concurrency();
	for (UINT i = 0; i < numThreads; ++i) {
		workerThreads.emplace_back(WorkerThread, hiocp);
	}
	for (auto& thread : workerThreads) {
		thread.join();
	}
	timerThread.join();

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
		EXP_OVER* expOverlapped = reinterpret_cast<EXP_OVER*>(overlapped);

		if (!ret) {
			if (expOverlapped->m_compType == COMP_TYPE::OP_ACCEPT) {
				cout << "OP_ACCEPT Error\n";
				exit(-1);
			}
			else {
				cout << "GetQueuedCompletionStatus Error on client[" << key << "]\n";
				// 접속 종료 패킷 전송

				g_sectorLock[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].lock();
				g_sector[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].erase(key);
				g_sectorLock[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].unlock();

				for (auto& pl : g_gameServer.GetClients()) {
					{
						unique_lock<mutex> lock(pl->m_mutex);
						if (pl->m_state != CLIENT::INGAME) continue;
					}
					if (pl->m_id == key) continue;
					pl->SendExitPlayer(key);
				}


				g_gameServer.ExitClient(key);
				if (expOverlapped->m_compType == OP_SEND) delete expOverlapped;
				continue;
			}
		}

		if ((received == 0) && ((expOverlapped->m_compType == OP_RECV) || (expOverlapped->m_compType == OP_SEND))) {
			// 접속 종료 패킷 전송

			g_sectorLock[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].lock();
			g_sector[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].erase(key);
			g_sectorLock[g_gameServer.GetClient(key)->m_position.y / (VIEW_RANGE * 2)][g_gameServer.GetClient(key)->m_position.x / (VIEW_RANGE * 2)].unlock();

			for (auto& pl : g_gameServer.GetClients()) {
				{
					unique_lock<mutex> lock(pl->m_mutex);
					if (pl->m_state != OBJECT::INGAME) continue;
				}
				if (pl->m_id == key) continue;
				pl->SendExitPlayer(key);
			}
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
				cout << "플레이어 연결 성공. ID : " << clientID << endl;
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
			// 패킷 재조립 필요
			size_t remainPacketSize = g_gameServer.GetClient(key)->m_prevRemain + received;
			char* remainPacketBuffer = expOverlapped->m_sendMsg;

			while (remainPacketSize > 0) {
				// 남은 데이터 사이즈가 0보다 클 시
				int packetSize = remainPacketBuffer[0];

				if (remainPacketSize < packetSize) {
					g_gameServer.GetClient(key)->m_prevRemain = remainPacketSize;
					if (remainPacketSize != 0) {
						memcpy(expOverlapped->m_sendMsg, remainPacketBuffer, remainPacketSize);
					}
					break;
				}
				// 하나의 온전한 패킷을 만들기 위한 사이즈보다
				// 모자랄 시 탈출

				ProcessPacket(key, remainPacketBuffer);
				// 패킷 처리

				remainPacketBuffer += packetSize;
				remainPacketSize -= packetSize;
				// 처리한 만큼 버퍼의 위치는 뒤로 조정
				// 버퍼의 크기는 줄임

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
		}
	}
}

void ProcessPacket(UINT cid, CHAR* packetBuf)
{
	switch (packetBuf[1])
	{
	case CS_PACKET_LOGIN:
	{
		// 새로 플레이어 들어옴
		cs_packet_login* pk = reinterpret_cast<cs_packet_login*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_LOGIN 수신" << endl;
#endif
		{
			sc_packet_login_confirm sendpk;
			sendpk.size = sizeof(sc_packet_login_confirm);
			sendpk.type = SC_PACKET_LOGIN_CONFIRM;
			sendpk.id = cid;
			g_gameServer.GetClient(cid)->DoSend(&sendpk);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_LOGIN_CONFIRM 송신 - ID : " << (int)sendpk.id << endl;
#endif
			{
				unique_lock<mutex> lock(g_gameServer.GetClient(cid)->m_mutex);
				g_gameServer.GetClient(cid)->m_state = OBJECT::INGAME;
			}

		}

		// 섹터 안에 있는 오브젝트의 ID를 담음.
		unordered_set<int> newViewList;
		short sectorX = g_gameServer.GetClient(cid)->m_position.x / (VIEW_RANGE * 2);
		short sectorY = g_gameServer.GetClient(cid)->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// 주변 9개의 섹터 전부 조사
		for (int i = 0; i < 9; ++i) {
			if (sectorX + dx[i] >= MAP_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= MAP_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
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
				if (!g_gameServer.CanSee(cid, id)) continue;

				// 일단 나한테 전송
				g_gameServer.GetClient(cid)->SendAddPlayer(id);
				// 상대는 NPC가 아닐 경우 전송
				if (id <= MAX_USER) g_gameServer.GetClient(id)->SendAddPlayer(cid);
			}
			g_sectorLock[sectorY + dy[i]][sectorX + dx[i]].unlock();
		}
		break;
	}
	case CS_PACKET_MOVE:
	{
		cs_packet_move* pk = reinterpret_cast<cs_packet_move*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_MOVE 수신" << endl;
#endif

		g_gameServer.Move(cid, (*pk).direction);
		g_gameServer.GetClient(cid)->m_lastMoveTime = pk->moveTime;
		
		// 섹터 안에 있는 오브젝트의 ID를 담음.
		unordered_set<int> newViewList;
		short sectorX = g_gameServer.GetClient(cid)->m_position.x / (VIEW_RANGE * 2);
		short sectorY = g_gameServer.GetClient(cid)->m_position.y / (VIEW_RANGE * 2);
		const array<INT, 9> dx = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		const array<INT, 9> dy = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		// 주변 9개의 섹터 전부 조사
		for (int i = 0; i < 9; ++i) {
			if (sectorX + dx[i] >= MAP_WIDTH / (VIEW_RANGE * 2) || sectorX + dx[i] < 0 ||
				sectorY + dy[i] >= MAP_HEIGHT / (VIEW_RANGE * 2) || sectorY + dy[i] < 0) {
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

		// 새 View List에 추가되었는데 이전에 없던 플레이어의 정보 추가
		for (auto& playerID : newViewList) {
			g_gameServer.GetClient(cid)->m_viewLock.lock();
			if (!g_gameServer.GetClient(cid)->m_viewList.count(playerID)) {
				// View List에 없다면 더해준다.
				g_gameServer.GetClient(cid)->m_viewLock.unlock();
				g_gameServer.GetClient(cid)->SendAddPlayer(playerID);
				g_gameServer.GetClient(playerID)->SendAddPlayer(cid);
			}
			else {
				// 있다면 이동 시킨다.
				g_gameServer.GetClient(cid)->m_viewLock.unlock();
				if (playerID <= MAX_USER) g_gameServer.GetClient(playerID)->SendObjectInfo(cid);
			}
		}
		
		// 새 View List에 제거되었는데 이전에 있던 플레이어의 정보 삭제
		g_gameServer.GetClient(cid)->m_viewLock.lock();
		auto oldViewList = g_gameServer.GetClient(cid)->m_viewList;
		g_gameServer.GetClient(cid)->m_viewLock.unlock();

		for (auto& playerID : oldViewList) {
			if (!newViewList.count(playerID)) {
				g_gameServer.GetClient(cid)->SendExitPlayer(playerID);
				if (playerID <= MAX_USER) g_gameServer.GetClient(playerID)->SendExitPlayer(cid);
			}
		}

		break;
	}
	}
}

void TimerThread(HANDLE hiocp)
{
	g_gameServer.TimerThread(hiocp);
}
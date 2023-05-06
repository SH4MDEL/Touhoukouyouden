#include "main.h"

int main()
{
	HANDLE hiocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(s_socket, SOMAXCONN);

	INT addr_size = sizeof(server_addr);

	hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(s_socket), hiocp, 9999, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER over;
	over.m_compType = COMP_TYPE::OP_ACCEPT;
	AcceptEx(s_socket, c_socket, over.m_sendMsg, 0, addr_size + 16, addr_size + 16, 0, &over.m_overlapped);

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
				for (auto& pl : g_gameServer.GetClients()) {
					if (pl.second.m_id == key) continue;
					sc_packet_exit_player packet;
					packet.size = sizeof(sc_packet_exit_player);
					packet.type = SC_PACKET_EXIT_PLAYER;
					packet.id = key;
					pl.second.DoSend(&packet);
				}
				g_gameServer.ExitClient(key);
				if (expOverlapped->m_compType == OP_SEND) delete expOverlapped;
				continue;
			}
		}

		switch (expOverlapped->m_compType)
		{
		case OP_ACCEPT:
		{
			UINT clientID = g_gameServer.RegistClient(c_socket);
			if (clientID != -1) {
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
					hiocp, clientID, 0);
				g_gameServer.GetClient(clientID).DoRecv();
#ifdef NETWORK_DEBUG
				cout << "플레이어 연결 성공. ID : " << clientID << endl;
#endif
			}
			else {
				cout << "MAX USER EXCEEDED.\n";
				closesocket(c_socket);
			}
			c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&over.m_overlapped, sizeof(over.m_overlapped));
			AcceptEx(s_socket, c_socket, over.m_sendMsg, 0, addr_size + 16, addr_size + 16, 0, &over.m_overlapped);
			break;
		}
		case OP_RECV:
		{
			// 패킷 재조립 필요
			size_t remainPacketSize = g_gameServer.GetClient(key).m_prevRemain + received;
			char* remainPacketBuffer = expOverlapped->m_sendMsg;

			while (remainPacketSize > 0) {
				// 남은 데이터 사이즈가 0보다 클 시
				int packetSize = remainPacketBuffer[0];

				if (remainPacketSize < packetSize) {
					g_gameServer.GetClient(key).m_prevRemain = remainPacketSize;
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
			g_gameServer.GetClient(key).DoRecv();
			break;
		}
		case OP_SEND:
		{
			delete expOverlapped;
			break;
		}
		}
	}

	g_gameServer.ResetClients();
	closesocket(s_socket);
	WSACleanup();
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
			sendpk.id = (CHAR)cid;
			g_gameServer.GetClient(cid).DoSend(&sendpk);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_LOGIN_CONFIRM 송신 - ID : " << (int)sendpk.id << endl;
#endif
		}

		// 새로 들어온 플레이어 정보 모든 플레이어에 전송
		sc_packet_add_player sendpk;
		sendpk.size = sizeof(sc_packet_add_player);
		sendpk.type = SC_PACKET_ADD_PLAYER;
		sendpk.id = (CHAR)cid;
		sendpk.coord = g_gameServer.GetClient(cid).m_position;
		for (auto& client : g_gameServer.GetClients()) {
			client.second.DoSend(&sendpk);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_ADD_PLAYER 송신 - ID : " << (int)sendpk.id << endl;
#endif
		}

		//  새로 들어온 플레이어에 자신의 정보를 제외한 모든 플레이어의 정보를 전송
		for (auto& client : g_gameServer.GetClients()) {
			if (client.second.m_id == cid) continue;
			sc_packet_add_player sendpk;
			sendpk.size = sizeof(sc_packet_add_player);
			sendpk.type = SC_PACKET_ADD_PLAYER;
			sendpk.id = client.second.m_id;
			sendpk.coord = g_gameServer.GetPlayerPosition(client.second.m_id);
			g_gameServer.GetClient(cid).DoSend(&sendpk);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_ADD_PLAYER 송신 - ID : " << (int)sendpk.id << endl;
#endif
		}
		break;
	}
	case CS_PACKET_MOVE:
	{
		cs_packet_move* pk = reinterpret_cast<cs_packet_move*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_MOVE 수신" << endl;
#endif
		sc_packet_object_info sendpk;
		sendpk.size = sizeof(sc_packet_object_info);
		sendpk.type = SC_PACKET_OBJECT_INFO;
		sendpk.id = (*pk).id;
		sendpk.coord = g_gameServer.Move((*pk).id, (*pk).direction);

		// 업데이트된 플레이어의 정보를 모든 플레이어에게 보냄
		for (auto& client : g_gameServer.GetClients()) {
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_OBJECT_INFO 송신 - ID : " << client.first << endl;
#endif
			client.second.DoSend(&sendpk);
		}
		break;
	}
	}
}
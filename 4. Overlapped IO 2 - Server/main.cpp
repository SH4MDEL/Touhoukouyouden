#include "main.h"

void CALLBACK SendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	delete reinterpret_cast<EXP_OVER*>(send_over);
}

void CALLBACK RecvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	unsigned long long sid = reinterpret_cast<unsigned long long>(recv_over->hEvent);
	packet* pk = reinterpret_cast<packet*>(g_gameServer.GetClient(sid).m_recvBuf);
	TranslatePacket((int)sid, pk);

	if (!num_bytes) {
		g_gameServer.ExitPlayer((int)sid);
		g_gameServer.ExitClient((int)sid);
		for (auto& client : g_gameServer.GetClients()) {
			sc_packet_exit_player* pk = reinterpret_cast<sc_packet_exit_player*>(g_gameServer.GetClient(client.first).m_recvBuf);
			(*pk).size = sizeof(sc_packet_exit_player);
			(*pk).type = SC_PACKET_EXIT_PLAYER;
			(*pk).id = sid;
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_EXIT_PLAYER 송신 - ID : " << client.first << endl;
#endif
			client.second.DoSend(client.first, (*pk).size, g_gameServer.GetClient(client.first).m_recvBuf);
		}
		return;
	}

	g_gameServer.GetClient(sid).DoRecv();
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(s_socket, SOMAXCONN);

	INT addr_size = sizeof(server_addr);
	for (int i = 1;; ++i) {
		SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
		g_gameServer.RegistClient(i, c_socket);
#ifdef NETWORK_DEBUG
		cout << "플레이어 연결 성공. ID : " << i << endl;
#endif
	}

	g_gameServer.ResetClients();
	closesocket(s_socket);
	WSACleanup();
}

void TranslatePacket(unsigned int sid, packet* packetBuf)
{
	switch (packetBuf->type)
	{
	case CS_PACKET_LOGIN:
	{
		// 새로 플레이어 들어옴
		cs_packet_login* pk = reinterpret_cast<cs_packet_login*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_LOGIN 수신" << endl;
#endif
		// 새로 들어온 플레이어 정보 서버에 추가
		sc_packet_login_confirm* sendpk = reinterpret_cast<sc_packet_login_confirm*>(g_gameServer.GetClient(sid).m_recvBuf);
		(*sendpk).id = g_gameServer.RegistPlayer(sid);
		if (sendpk->id != -1) {
			sendpk->size = sizeof(sc_packet_login_confirm);
			sendpk->type = SC_PACKET_LOGIN_CONFIRM;
			g_gameServer.GetClient(sid).DoSend(sid, (*sendpk).size, g_gameServer.GetClient(sid).m_recvBuf);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_LOGIN_CONFIRM 송신" << endl;
#endif
		}
		else return;


		// 새로 들어온 플레이어의 정보 모든 플레이어에 전송
		for (const auto& client : g_gameServer.GetClients()) {
			sc_packet_add_player* sendpk = reinterpret_cast<sc_packet_add_player*>(g_gameServer.GetClient(sid).m_recvBuf);
			sendpk->size = sizeof(sc_packet_add_player);
			sendpk->type = SC_PACKET_ADD_PLAYER;
			sendpk->id = (char)sid;
			sendpk->coord = g_gameServer.GetPlayer(sid);
			g_gameServer.GetClient(client.first).DoSend(client.first, (*sendpk).size, g_gameServer.GetClient(sid).m_recvBuf);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_ADD_PLAYER 송신" << endl;
#endif
		}

		//  새로 들어온 플레이어에 자신의 정보를 제외한 모든 플레이어의 정보를 전송
		for (const auto& player : g_gameServer.GetPlayers()) {
			if (player.first == sid) continue;
			sc_packet_add_player* sendpk = reinterpret_cast<sc_packet_add_player*>(g_gameServer.GetClient(sid).m_recvBuf);
			sendpk->size = sizeof(sc_packet_add_player);
			sendpk->type = SC_PACKET_ADD_PLAYER;
			sendpk->id = (char)player.first;
			sendpk->coord = player.second;
			g_gameServer.GetClient(sid).DoSend(sid, (*sendpk).size, g_gameServer.GetClient(sid).m_recvBuf);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_ADD_PLAYER 송신" << endl;
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
		int direction = pk->direction;
		int moveTime = pk->moveTime;
		sc_packet_object_info* sendpk = reinterpret_cast<sc_packet_object_info*>(g_gameServer.GetClient(sid).m_recvBuf);
		sendpk->size = sizeof(sc_packet_object_info);
		sendpk->type = SC_PACKET_OBJECT_INFO;
		sendpk->id = sid;
		sendpk->coord = g_gameServer.Move(sid, direction);
		//cout << (int)pk->direction << endl;
		sendpk->moveTime = moveTime;

		// 업데이트된 플레이어의 정보를 모든 플레이어에게 보냄
		for (auto& client : g_gameServer.GetClients()) {
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_OBJECT_INFO 송신 - ID : " << client.first << endl;
#endif
			client.second.DoSend(client.first, sendpk->size, g_gameServer.GetClient(sid).m_recvBuf);
		}
		break;
	}
	default:
		break;
	}
}
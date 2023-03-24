#include "main.h"

void CALLBACK SendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	delete reinterpret_cast<EXP_OVER*>(send_over);
}

void CALLBACK RecvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	unsigned long long sid = reinterpret_cast<unsigned long long>(recv_over->hEvent);
	packet* pk = reinterpret_cast<packet*>(g_gameServer.GetClient(sid).m_recvBuf);
	TranslatePacket(sid, pk);

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

void TranslatePacket(unsigned long long sid, packet* packetBuf)
{
	switch ((*packetBuf).type)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login* pk = reinterpret_cast<cs_packet_login*>(packetBuf);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_LOGIN 수신" << endl;
#endif
		sc_packet_login_confirm* sendpk = reinterpret_cast<sc_packet_login_confirm*>(g_gameServer.GetClient(sid).m_recvBuf);
		(*sendpk).id = g_gameServer.RegistPlayer(sid);
		if ((*sendpk).id != -1) {
			(*sendpk).size = sizeof(sc_packet_login_confirm);
			(*sendpk).type = SC_PACKET_LOGIN_CONFIRM;
			g_gameServer.GetClient(sid).DoSend(sid, (*sendpk).size, g_gameServer.GetClient(sid).m_recvBuf);
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_LOGIN_CONFIRM 송신" << endl;
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
		sc_packet_object_info* sendpk = reinterpret_cast<sc_packet_object_info*>(g_gameServer.GetClient(sid).m_recvBuf);
		(*sendpk).coord = g_gameServer.Move((*pk).id, (*pk).coord);
		(*sendpk).size = sizeof(sc_packet_object_info);
		(*sendpk).type = SC_PACKET_OBJECT_INFO;
		for (auto& client : g_gameServer.GetClients()) {
#ifdef NETWORK_DEBUG
			cout << "SC_PACKET_OBJECT_INFO 송신 - ID : " << client.first << endl;
#endif
			client.second.DoSend(sid, (*sendpk).size, g_gameServer.GetClient(sid).m_recvBuf);
		}
		break;
	}
	default:
		break;
	}
}
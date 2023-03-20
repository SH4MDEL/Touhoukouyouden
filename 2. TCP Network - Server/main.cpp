#include "main.h"

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(s_socket, SOMAXCONN);

	INT addr_size = sizeof(server_addr);
	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);

#ifdef NETWORK_DEBUG
	cout << "플레이어 연결 성공" << endl;
#endif

	while(Recv(c_socket)) {}

	WSACleanup();
}

void Send(SOCKET socket, void* packetBuf)
{
	int remain, retval;
	remain = reinterpret_cast<packet*>(packetBuf)->size;
	while (remain > 0) {
		retval = send(socket, reinterpret_cast<char*>(packetBuf) + reinterpret_cast<packet*>(packetBuf)->size - remain, remain, 0);
		remain -= retval;
	}
}


int Recv(SOCKET socket)
{
	packet pk;
	int retval = recv(socket, reinterpret_cast<char*>(&pk), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		std::cout << "Socket Error in recv" << std::endl;
		return 0;
	}
	else {
		TranslatePacket(socket, pk);
		return 1;
	}
}

void TranslatePacket(SOCKET socket, const packet& packetBuf)
{
	int retval, remain;
	switch (packetBuf.type)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvpk;
		retval = recv(socket, reinterpret_cast<char*>(&recvpk) + 2, packetBuf.size - 2, MSG_WAITALL);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_LOGIN 수신" << endl;
#endif
		sc_packet_login_confirm sendpk;
		sendpk.id = g_gameServer.RegistPlayer();
		if (sendpk.id != -1) {
			sendpk.size = sizeof(sc_packet_login_confirm);
			sendpk.type = SC_PACKET_LOGIN_CONFIRM;
			Send(socket , &sendpk);
		}
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_LOGIN_CONFIRM 송신" << endl;
#endif
		break;
	}
	case CS_PACKET_MOVE:
	{
		cs_packet_move recvpk;
		retval = recv(socket, reinterpret_cast<char*>(&recvpk) + 2, packetBuf.size - 2, MSG_WAITALL);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_MOVE 수신" << endl;
#endif
		sc_packet_object_info sendpk;
		sendpk.coord = g_gameServer.Move(recvpk.id, recvpk.coord);
		sendpk.size = sizeof(sc_packet_object_info);
		sendpk.type = SC_PACKET_OBJECT_INFO;
		Send(socket, &sendpk);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_OBJECT_INFO 송신" << endl;
#endif
		break;
	}
	}
}
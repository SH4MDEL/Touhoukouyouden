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

	while(Recv(c_socket)) {}

	WSACleanup();
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
		break;
	}
	case CS_PACKET_MOVE:
	{
		break;
	}
	}
}
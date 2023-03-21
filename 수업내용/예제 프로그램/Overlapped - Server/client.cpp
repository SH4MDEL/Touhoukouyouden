#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")
using namespace std;

constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 200;

// 전역변수를 통해 Send/Recv가 공유
SOCKET client;
WSAOVERLAPPED c_over;
WSABUF c_wsabuf[1];
CHAR c_mess[BUF_SIZE];

void do_recv();

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, NULL, NULL);
	do_recv();
	// do recv 실시 후 논다. (SleepEx : Callback 함수를 실행하는 시스템 콜)
	// 아무튼 시스템 콜이 발생한다. 클라이언트에서는 Non-blocking을 사용하자.
	// 모든 컨텐츠는 callback 함수 내에서 실행된다.
	while (true) SleepEx(100, true);
	closesocket(server);
	WSACleanup();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void do_recv()
{
	c_wsabuf[0].buf = c_mess;
	c_wsabuf[0].len = BUF_SIZE;
	DWORD recv_flag = 0;
	memset(&c_over, 0, sizeof(c_over));
	// 클라이언트가 데이터를 줘서 recv_callback이 동작하면 서버가 작동.
	WSARecv(client, c_wsabuf, 1, 0, &recv_flag, &c_over, recv_callback);
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	do_recv();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	if (0 == num_bytes) return;
	cout << "Client sent: " << c_mess << endl;
	c_wsabuf[0].len = num_bytes;
	memset(&c_over, 0, sizeof(c_over));
	WSASend(client, c_wsabuf, 1, 0, 0, &c_over, send_callback);
}


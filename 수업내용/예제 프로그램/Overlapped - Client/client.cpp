#include <iostream>
#include <WS2tcpip.h>
using namespace std;
#pragma comment(lib, "WS2_32.LIB")

constexpr short SERVER_PORT = 3500;
constexpr int BUF_SIZE = 200;

WSAOVERLAPPED s_over;
SOCKET s_socket;
WSABUF s_wsabuf[1];
char s_buf[BUF_SIZE];

void do_send_message();

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);

	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);
	WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);

	do_send_message();

	while (true) SleepEx(100, true);

	closesocket(s_socket);
	WSACleanup();
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

void do_send_message()
{
	cout << "Enter Messsage: ";
	cin.getline(s_buf, BUF_SIZE - 1);
	s_wsabuf[0].buf = s_buf;
	s_wsabuf[0].len = static_cast<int>(strlen(s_buf)) + 1;
	memset(&s_over, 0, sizeof(s_over));
	WSASend(s_socket, s_wsabuf, 1, 0, 0, &s_over, send_callback);
}
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	cout << "Server Sent: " << s_buf << endl;
	do_send_message();
}
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	s_wsabuf[0].len = BUF_SIZE;
	DWORD r_flag = 0;
	memset(over, 0, sizeof(*over));
	WSARecv(s_socket, s_wsabuf, 1, 0, &r_flag, over, recv_callback);
}
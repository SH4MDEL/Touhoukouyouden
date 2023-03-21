#include <iostream>
#include <WS2tcpip.h>
#include <unordered_map>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")

const short SERVER_PORT = 3500;
const int BUFSIZE = 256;

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag);
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);

class SESSION {
private:
	size_t _id;
	WSABUF _recv_wsabuf;
	WSABUF _send_wsabuf;
	WSAOVERLAPPED _recv_over;
	SOCKET _socket;

public:
	char _recv_buf[BUFSIZE];
	SESSION() {
		cout << "Unexpected Constructor Call Error!\n";
		exit(-1);
	}
	SESSION(int id, SOCKET s) : _id(id), _socket(s) {
		_recv_wsabuf.buf = _recv_buf; _recv_wsabuf.len = BUFSIZE;
		_send_wsabuf.buf = _recv_buf; _send_wsabuf.len = 0;
	}
	~SESSION() { closesocket(_socket); }

	void do_recv() {
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over, sizeof(_recv_over));
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		WSARecv(_socket, &_recv_wsabuf, 1, 0, &recv_flag, &_recv_over, recv_callback);
	}

	void do_send(int num_bytes) {
		ZeroMemory(&_recv_over, sizeof(_recv_over));
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		_send_wsabuf.len = num_bytes;
		WSASend(_socket, &_send_wsabuf, 1, 0, 0, &_recv_over, send_callback);
	}
};

unordered_map <size_t, SESSION> clients;

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	size_t s_id = reinterpret_cast<size_t>(send_over->hEvent);
	clients[s_id].do_recv();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	size_t s_id = reinterpret_cast<size_t>(recv_over->hEvent);
	cout << "Client Sent [" << num_bytes << "bytes] : " << clients[s_id]._recv_buf << endl;
	clients[s_id].do_send(num_bytes);
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
	for (int i = 1; ; ++i) {
		SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
		clients.try_emplace(i, i, c_socket);
		clients[i].do_recv();
	}

	clients.clear();
	closesocket(s_socket);
	WSACleanup();
}
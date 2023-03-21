#include <iostream>
#include <WS2tcpip.h>
#include <unordered_map>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")
const short SERVER_PORT = 4000;
const int BUFSIZE = 256;


void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag);
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);

class EXP_OVER {
public:
	WSAOVERLAPPED _wsa_over;
	unsigned long long _s_id;
	WSABUF _wsa_buf;
	char _send_msg[BUFSIZE];
public:
	EXP_OVER(unsigned long long s_id, char num_bytes, const char* mess) : _s_id(s_id)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = _send_msg;
		_wsa_buf.len = num_bytes + 2;

		memcpy(_send_msg + 2, mess, num_bytes);
		_send_msg[0] = num_bytes + 2;
		_send_msg[1] = static_cast<char>(s_id);
	}

	~EXP_OVER() {}
};

class SESSION {
private:
	unsigned long long   _id;
	WSABUF _recv_wsabuf;
	WSABUF _send_wsabuf;
	WSAOVERLAPPED _recv_over;
	SOCKET _socket;
public:
	char   _recv_buf[BUFSIZE];
	SESSION() {
		cout << "Unexpected Constructor Call Error!\n";
		exit(-1);
	}
	SESSION(int id, SOCKET s) : _id(id), _socket(s) {
		_recv_wsabuf.buf = _recv_buf;    _recv_wsabuf.len = BUFSIZE;
		_send_wsabuf.buf = _recv_buf;    _send_wsabuf.len = 0;
	}
	~SESSION() { closesocket(_socket); }
	void do_recv() {
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over, sizeof(_recv_over));
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		WSARecv(_socket, &_recv_wsabuf, 1, 0, &recv_flag, &_recv_over, recv_callback);
	}

	void do_send(unsigned long long sender_id, int num_bytes, const char* buff) {
		EXP_OVER* send_over = new EXP_OVER(sender_id, num_bytes, buff);
		WSASend(_socket, &send_over->_wsa_buf, 1, 0, 0, &send_over->_wsa_over, send_callback);
	}
};

unordered_map <unsigned long long, SESSION> clients;

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	delete reinterpret_cast<EXP_OVER*>(send_over);
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	unsigned long long s_id = reinterpret_cast<unsigned long long>(recv_over->hEvent);
	cout << "Client [" << s_id << "] Sent[" << num_bytes << "bytes] : " << clients[s_id]._recv_buf << endl;
	for (auto& cl : clients)
		cl.second.do_send(s_id, num_bytes, clients[s_id]._recv_buf);
	clients[s_id].do_recv();
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


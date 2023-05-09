#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <chrono>
#include <queue>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

constexpr int VIEW_RANGE = 4;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_AI };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class SESSION {
	OVER_EXP _recv_over;		// NPC에게는 필요 없음

public:
	mutex _s_lock;
	S_STATE _state;
	bool _is_active;			// 플레이어에게 필요 없음.
	int _id;
	SOCKET _socket;				// NPC에게는 필요 없음
	short	x, y;
	char	_name[NAME_SIZE];
	unordered_set<int> view_list;	// NPC에게는 필요 없음.
	mutex	_vl;
	int		_prev_remain;
	int		_last_move_time;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_remove_player_packet(int c_id)
	{
		_vl.lock();
		if (view_list.count(c_id) == 0) {
			_vl.unlock();
			return;
		}
		view_list.erase(c_id);
		_vl.unlock();
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	}
};

array<SESSION, MAX_USER + MAX_NPC> clients;

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;


enum EVENT_TYPE { EV_RANDOM_MOVE, EV_HEAL, EV_ATTACK };
struct EVENT
{
	int _oid;
	chrono::system_clock::time_point _exec_time;
	EVENT_TYPE _type;

	constexpr bool operator<(const EVENT& left) const
	{
		return (_exec_time > left._exec_time);
	}
};

priority_queue<EVENT> timer_queue;
mutex g_tl;

bool can_see(int a, int b)
{
	//return VIEW_RANGE * VIEW_RANGE >= (clients[a].x - clients[b].x) * (clients[a].x - clients[b].x) +
	//	(clients[a].y - clients[b].y) * (clients[a].y - clients[b].y);

	if (std::abs(clients[a].x - clients[b].x) > VIEW_RANGE) return false;
	return std::abs(clients[a].y - clients[b].y <= VIEW_RANGE);
}

bool is_pc(int id)
{
	return id < MAX_USER;
}

void add_timer(int o_id, EVENT_TYPE et, chrono::system_clock::time_point exec_t)
{
	EVENT n_ev{ o_id, exec_t, et };
	g_tl.lock();
	timer_queue.push(n_ev);
	g_tl.unlock();
}

bool wakeup_npc(int id)
{
	add_timer(id, EVENT_TYPE::EV_RANDOM_MOVE, chrono::system_clock::now() + 1s);
}

void SESSION::send_move_packet(int c_id)
{
	_vl.lock();
	if (view_list.count(c_id) == 0) {
		_vl.unlock();
		send_add_player_packet(c_id);
		return;
	}
	_vl.unlock();
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.move_time = clients[c_id]._last_move_time;
	do_send(&p);
}

void SESSION::send_add_player_packet(int c_id)
{
	_vl.lock();
	if (view_list.count(c_id) != 0 || c_id == _id) {
		_vl.unlock();
		send_move_packet(c_id);
		return;
	}
	view_list.insert(c_id);
	_vl.unlock();

	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.x = clients[c_id].x;
	add_packet.y = clients[c_id].y;
	do_send(&add_packet);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].x = rand() % W_WIDTH;
		clients[c_id].y = rand() % W_HEIGHT;
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id]._s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			if (!can_see(pl._id, c_id)) continue;

			if (is_pc(pl._id)) pl.send_add_player_packet(c_id);	// npc에는 add 할 필요 없음.
			else wakeup_npc(pl._id);
			clients[c_id].send_add_player_packet(pl._id);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]._last_move_time = p->move_time;
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;

		unordered_set<int> new_vl;
		for (auto& cl : clients) {
			if (cl._id == c_id) continue;
			if (cl._state != ST_INGAME) continue;
			if (!can_see(cl._id, c_id)) continue;

			new_vl.insert(cl._id);
		}

		clients[c_id].send_move_packet(c_id);

		// 시야 추가 (new_vl에 있는데 old_vl에는 없던 플레이어)
		for (auto& pl : new_vl) {
			if (is_pc(pl)) {
				clients[c_id]._vl.lock();
				if (0 == clients[c_id].view_list.count(clients[pl]._id)) {
					clients[c_id]._vl.unlock();
					clients[pl].send_add_player_packet(c_id);
					clients[c_id].send_add_player_packet(pl);
				}
				// 이동 처리
				else {
					clients[c_id]._vl.unlock();
					clients[pl].send_move_packet(c_id);

				}
			}
		}

		// 시야 제거 (옛날엔 있었는데 지금은 없음)
		clients[c_id]._vl.lock();
		auto old_vl = clients[c_id].view_list;
		clients[c_id]._vl.unlock();

		for (auto& pl : old_vl) {
			if (new_vl.count(pl) == 0) {
				clients[c_id].send_remove_player_packet(pl);
				if (is_pc(pl)) clients[pl].send_remove_player_packet(c_id);
			}
		}
	}
	}
}

void disconnect(int c_id)
{
	for (auto& pl : clients) {
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		// pc일 경우에만 send
		if (is_pc(pl._id)) {
			pl.send_remove_player_packet(c_id);
		}
	}
	closesocket(clients[c_id]._socket);

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	clients[c_id]._state = ST_FREE;
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND: {
			delete ex_over;
			break;
		}
		case OP_NPC_AI: {
			do_random_move(key);
			bool deactivate = true;
			for (int i = 0; i < MAX_USER; ++i) {
				if (clients[i]._state != ST_INGAME) continue;
				if (!can_see(i, key)) continue;
				deactivate = false;
				break;
			}
			if (!deactivate) {
				add_timer(key, EV_RANDOM_MOVE, chrono::system_clock::now() + 1s);
			}
			break;
		}
		}
	}
}

void InitializeNPC()
{
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		clients[i].x = rand() % W_WIDTH;
		clients[i].y = rand() & W_HEIGHT;
		clients[i]._id = i;
		sprintf_s(clients[i]._name, "N%d", i);
		clients[i]._state = ST_INGAME;
	}
}

void do_random_move(int id)
{
	unordered_set<int> view_list;
	for (auto& cl : clients) {
		if (cl._id >= MAX_USER) break;
		if (cl._state != ST_INGAME) continue;
		if (cl._id == id) continue;
		if (!can_see(cl._id, id)) continue;
		view_list.insert(cl._id);
	}

	short x = clients[id].x;
	short y = clients[id].y;
	switch (rand() % 4) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}
	clients[id].x = x;
	clients[id].y = y;

	unordered_set<int> near_list;
	for (auto& cl : clients) {
		if (cl._id >= MAX_USER) break;
		if (cl._id == id) continue;
		if (cl._state != ST_INGAME) continue;
		if (!can_see(cl._id, id)) continue;

		near_list.insert(cl._id);

	}

	// 시야 추가 (near_list에 있는데 이전에는 없던 플레이어)
	for (auto& pl : near_list) {
		if (is_pc(pl)) {
			clients[id]._vl.lock();
			if (0 == clients[id].view_list.count(clients[pl]._id)) {
				clients[id]._vl.unlock();
				clients[pl].send_add_player_packet(id);
				clients[id].send_add_player_packet(pl);
			}
			// 이동 처리
			else {
				clients[id]._vl.unlock();
				clients[pl].send_move_packet(id);

			}
		}
	}

	// 시야 제거 (옛날엔 있었는데 지금은 없음)
	for (auto& pl : view_list) {
		if (view_list.count(pl) == 0) {
			if (is_pc(pl)) clients[pl].send_remove_player_packet(id);
		}
	}
}

void do_ai()
{
	using namespace chrono;
	while (true) {
		auto start_time = system_clock::now();
		for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
			do_random_move(i);
		}
		auto end_time = system_clock::now();
		auto ai_time = end_time - start_time;
		this_thread::sleep_for(1s - ai_time);
	}
}

void do_timer(HANDLE hiocp)
{
	while (true)
	{
		g_tl.lock();
		if (timer_queue.empty()) {
			g_tl.unlock();
			this_thread::sleep_for(10ms);
			continue;
		}
		auto ev = timer_queue.top();
		if (ev._exec_time > chrono::system_clock::now()) {
			g_tl.unlock();
			this_thread::sleep_for(10ms);
			continue;
		}
		// do ai
		timer_queue.pop();
		g_tl.unlock();
		OVER_EXP* exover = new OVER_EXP;
		exover->_comp_type = OP_NPC_AI;
		// type 제외하고 초기화할 필요 없음
		PostQueuedCompletionStatus(hiocp, 1, ev._oid, &exover->_over);
	}
}

int main()
{
	HANDLE h_iocp;

	InitializeNPC();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	//thread ai_thread{ do_ai };
	thread timer_thread{ do_timer, h_iocp };

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	//ai_thread.join();
	timer_thread.join();
	closesocket(g_s_socket);
	WSACleanup();
}

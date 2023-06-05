#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <chrono>
#include <queue>
#include <atomic>
#include <concurrent_unordered_map.h>

#include "protocol.h"
#include <boost/asio.hpp>


using namespace std;
using namespace chrono;
using boost::asio::ip::tcp;

atomic_int g_user_ID;

const auto X_START_POS = 4;
const auto Y_START_POS = 4;

class session;


// shared_ptr <- 멀티쓰레드 자료구조가 아니다.
// 읽고 쓰고 하면 Data Race
// 그러면 atomic_shared_ptr<session> 하면 session 내 멤버변수를 마음대로 수정해도 atomic하게 동작?
// 너무 개사긴데

concurrency::concurrent_unordered_map<int, shared_ptr<session>> players;

void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}


int GetNewClientID()
{
	if (g_user_ID >= MAX_USER) {
		cout << "MAX USER FULL\n";
		exit(-1);
	}
	return g_user_ID++;
}

class session
	: public std::enable_shared_from_this<session>
{
private:
	tcp::socket socket_;
	int my_id_;
	enum { max_length = 1024 };
	unsigned char data_[max_length];
	unsigned char packet_[max_length];	
	int curr_packet_size_;
	int prev_data_size_;

	int pos_x;
	int pos_y;


	void Send_Packet(void* packet, unsigned id)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		players[id]->do_write(buff, packet_size);
	}

	void Process_Packet(unsigned char* packet, int id)
	{
		auto P = players[id];
		int y = P->pos_y;
		int x = P->pos_x;
		switch (packet[1]) {
		case CS_UP: y--; if (y < 0) y = 0; break;
		case CS_DOWN: y++; if (y >= BOARD_HEIGHT) y = BOARD_HEIGHT - 1; break;
		case CS_LEFT: x--; if (x < 0) x = 0; break;
		case CS_RIGHT: x++; if (x >= BOARD_WIDTH) x = BOARD_WIDTH - 1; break;
		default: cout << "Invalid Packet From Client [" << id << "]\n"; system("pause"); exit(-1);
		}
		P->pos_x = x;
		P->pos_y = y;

		sc_packet_pos sp;

		sp.id = id;
		sp.size = sizeof(sc_packet_pos);
		sp.type = SC_POS;
		sp.x = P->pos_x;
		sp.y = P->pos_y;

		for (auto& pl : players)
			pl.second->Send_Packet(&sp);
	}

	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				if (ec)
				{
					if (ec.value() == boost::asio::error::operation_aborted) return;
					cout << "Receive Error on Session[" << my_id_ << "] EC[" << ec << "]\n";
					players[my_id_] = nullptr;
					//players.unsafe_erase(my_id_);
					return;
				}

				int data_to_process = static_cast<int>(length);
				unsigned char* buf = data_;
				while (0 < data_to_process) {
					if (0 == curr_packet_size_) {
						curr_packet_size_ = buf[0];
						if (buf[0] > 200) {
							cout << "Invalid Packet Size [ << buf[0] << ] Terminating Server!\n";
							exit(-1);
						}
					}
					int need_to_build = curr_packet_size_ - prev_data_size_;
					if (need_to_build <= data_to_process) {
						// 패킷 조립
						memcpy(packet_ + prev_data_size_, buf, need_to_build);
						Process_Packet(packet_, my_id_);
						curr_packet_size_ = 0;
						prev_data_size_ = 0;
						data_to_process -= need_to_build;
						buf += need_to_build;
					}
					else {
						// 훗날을 기약
						memcpy(packet_ + prev_data_size_, buf, data_to_process);
						prev_data_size_ += data_to_process;
						data_to_process = 0;
						buf += data_to_process;
					}
				}
				do_read();
			});
	}

	void do_write(unsigned char* packet, std::size_t length)
	{
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(packet, length),
			[this, self, packet, length](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{
					if (length != bytes_transferred) {
						cout << "Incomplete Send occured on session[" << my_id_ << "]. This session should be closed.\n";
					}
					delete packet;
				}
			});
	}

public:
	session(tcp::socket socket, int new_id)
		: socket_(std::move(socket)), my_id_(new_id)
	{
		curr_packet_size_ = 0;
		prev_data_size_ = 0;
	}

	void start()
	{
		do_read();

		sc_packet_login_info pl;
		pl.id = my_id_;
		pl.size = sizeof(sc_packet_login_info);
		pl.type = SC_LOGIN_INFO;
		pl.x = pos_x;
		pl.y = pos_y;
		Send_Packet(&pl);

		sc_packet_put_player p;
		p.id = my_id_;
		p.size = sizeof(sc_packet_put_player);
		p.type = SC_PUT_PLAYER;
		p.x = pos_x;
		p.y = pos_y;

		// 나의 접속을 모든 플레이어에게 알림
		for (auto& pl : players)
			if (pl.second != nullptr)
				pl.second->Send_Packet(&p);
		// 나에게 접속해 있는 다른 플레이어 정보를 전송
		// 나에게 주위에 있는 NPC의 정보를 전송
		for (auto& pl : players) {
			if (pl.second->my_id_ != my_id_) {
				p.id = pl.second->my_id_;
				p.x = pl.second->pos_x;
				p.y = pl.second->pos_y;
				Send_Packet(&p);
			}
		}
	}

	void Send_Packet(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		do_write(buff, packet_size);
	}
};

class server
{
private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					int p_id = GetNewClientID();
					players[p_id] = std::make_shared<session>(std::move(socket_), p_id);
					players[p_id]->start();
					do_accept();
				}
			});
	}

public:
	server(boost::asio::io_context& io_service, int port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		socket_(io_service)
	{
		do_accept();
	}
};

void worker_thread(boost::asio::io_context* service)
{
	service->run();
}

int main()
{
	boost::asio::io_context io_service;
	vector <thread > worker_threads;
	server s(io_service, MY_SERVER_PORT);

	Init_Server();

	for (auto i = 0; i < 4; i++) worker_threads.emplace_back(worker_thread, &io_service);
	for (auto& th : worker_threads) th.join();
}
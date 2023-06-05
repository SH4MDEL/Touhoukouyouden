#include <iostream>
#include <sdkddkver.h>
#include <unordered_map>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;
constexpr int PORT = 3500;
constexpr int max_length = 1024;

class session;
unordered_map <int, session> g_clients;
class session
{
	int my_id;
	tcp::socket socket_;
	char data_[max_length];
public:
	session() : socket_(nullptr) {
		cout << "Session Creation Error.\n";
	}
	session(tcp::socket socket, int id) : socket_(std::move(socket)), my_id(id) {	// 소켓 객체는 복사 불가능하므로 이동
		do_read();	// 소멸자 호출될때 커널과 동반자살하기때문에 함부로 복사할 수 없다.
					// 소켓이 그냥 인트가 아니라 객체로 관리되기때문에
	}
	void do_read() {
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this](boost::system::error_code ec, std::size_t length) {	// 어떤 소켓이 완료되었는지 알 수가 없으니까
				if (ec) g_clients.erase(my_id);							// 람다를 통해 my_id까지 같이 넘겨서 지워준다.
				else g_clients[my_id].do_write(length); });
	}
	void do_write(std::size_t length) {
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this](boost::system::error_code ec, std::size_t /*length*/) {
				if (!ec)g_clients[my_id].do_read();
				else g_clients.erase(my_id); });
	}
};

int g_client_id = 0;
void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor);

int main(int argc, char* argv[])
{
	try {
		boost::asio::io_context io_context;
		tcp::acceptor my_acceptor{ io_context, tcp::endpoint(tcp::v4(), PORT) };
		my_acceptor.async_accept([&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
			accept_callback(ec, socket, my_acceptor); });	// 계속 async_accept를 실시하기 위해 인자로 my_acceptor 전달 (재귀)
		io_context.run(); // run()을 해 줘야 handler가 호출이 됨
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor)
{
	g_clients.try_emplace(g_client_id, move(socket), g_client_id);
	g_client_id++;
	my_acceptor.async_accept([&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
		accept_callback(ec, socket, my_acceptor);
		});
}
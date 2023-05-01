#include "main.h"

#include "mainScene.h"

void InitInstance();

int main()
{
	wcout.imbue(locale("korean"));
    sf::Socket::Status status = g_socket.connect("127.0.0.1", SERVER_PORT);
	g_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		exit(-1);
	}

	if (!g_font.loadFromFile("..\\Resource\\cour.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}

	InitInstance();

	g_window = make_shared<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Overlapped I/O 2");

    cs_packet_login packet;
    packet.size = sizeof(cs_packet_login);
    packet.type = CS_PACKET_LOGIN;
    Send(&packet);
#ifdef NETWORK_DEBUG
    cout << "CS_PACKET_LOGIN 송신" << endl;
#endif
	
	while (g_window->isOpen()) {
        sf::Event event;
		while (g_window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				g_window->close();
			}
			if (event.type == sf::Event::KeyPressed) {
				g_gameFramework.OnProcessingKeyboardMessage(event);
			}
		}
		g_window->clear();
		g_gameFramework.FrameAdvance();
		g_window->display();
	}
}

void InitInstance()
{
	g_gameFramework.OnCreate();
}

void Send(void* packetBuf)
{
    unsigned char* packet = reinterpret_cast<unsigned char*>(packetBuf);
    size_t sent = 0;
    g_socket.send(packetBuf, packet[0], sent);
}

void Recv()
{
    char buf[BUFSIZE];
    size_t received;
    auto recvResult = g_socket.receive(buf, BUFSIZE, received);

    if (recvResult == sf::Socket::Error) {
#ifdef NETWORK_DEBUG
        cout << "Recv Error!" << endl;
        exit(-1);
#endif // NETWORK_DEBUG
    }
    if (recvResult == sf::Socket::Disconnected) {
#ifdef NETWORK_DEBUG
        cout << "Disconnected!" << endl;
        exit(-1);
#endif // NETWORK_DEBUG
    }
    if (recvResult != sf::Socket::NotReady) {
        if (received > 0) {
            TranslatePacket(buf, received);
        }
    }
}

void TranslatePacket(char* buf, size_t io_byte)
{
    char* packet = buf;
    static size_t remainPacketSize = 0;
    static char remainPacketBuffer[BUFSIZE];

    memcpy(remainPacketBuffer + remainPacketSize, packet, io_byte);
    // 이전 남은 버퍼에 이번에 받은 패킷을 이어 붙임
    remainPacketSize += io_byte;
    // 이번에 받은 데이터 사이즈만큼 더함
    while (remainPacketSize > 0) {
        // 남은 데이터 사이즈가 0보다 클 시
        int packetSize = remainPacketBuffer[0];

        if (remainPacketSize < packetSize) break;
        // 하나의 온전한 패킷을 만들기 위한 사이즈보다
        // 모자랄 시 탈출

        ProcessPacket(remainPacketBuffer);
        // 패킷 처리

        packet += packetSize;
        remainPacketSize -= packetSize;
        if (remainPacketSize != 0) {
            memcpy(remainPacketBuffer, packet, remainPacketSize);
        }
    }
}

void ProcessPacket(char* buf)
{
    switch (buf[1])
    {
    case SC_PACKET_LOGIN_CONFIRM:
    {
        sc_packet_login_confirm* pk = reinterpret_cast<sc_packet_login_confirm*>(buf);
        g_clientID = pk->id;
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_LOGIN_CONFIRM 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_ADD_PLAYER:
    {
        sc_packet_add_player* pk = reinterpret_cast<sc_packet_add_player*>(buf);
        MainScene* scene = (MainScene*)g_gameFramework.GetScene();
        scene->AddPlayer(pk->id, pk->coord);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_ADD_PLAYER 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_OBJECT_INFO:
    {
        sc_packet_object_info* pk = reinterpret_cast<sc_packet_object_info*>(buf);
        MainScene* scene = (MainScene*)g_gameFramework.GetScene();
        scene->Move(pk->id, pk->coord);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_OBJECT_INFO 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_EXIT_PLAYER:
    {
        sc_packet_exit_player* pk = reinterpret_cast<sc_packet_exit_player*>(buf);
        MainScene* scene = (MainScene*)g_gameFramework.GetScene();
        scene->ExitPlayer(pk->id);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_EXIT_PLAYER 수신" << endl;
#endif
        break;
    }
    }
}

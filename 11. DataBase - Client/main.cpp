#include "main.h"

#include "mainScene.h"

void InitInstance();

int main()
{
	InitInstance();

	g_window = make_shared<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "DataBase");
	
	while (g_window->isOpen()) {
        sf::Event event;
		while (g_window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				g_window->close();
			}
			if (event.type == sf::Event::TextEntered ||
                event.type == sf::Event::KeyPressed ||
                event.type == sf::Event::KeyReleased) {
				g_gameFramework.OnProcessingKeyboardMessage(event);
			}
            if (event.type == sf::Event::MouseButtonPressed ||
                event.type == sf::Event::MouseButtonReleased ||
                event.type == sf::Event::MouseMoved) {
                g_gameFramework.OnProcessingMouseMessage(event, g_window);
            }
		}
		g_window->clear();
		g_gameFramework.FrameAdvance();
		g_window->display();
	}
}

void InitInstance()
{
    if (!g_font.loadFromFile("..\\Resource\\cour.ttf")) {
        cout << "Font Loading Error!\n";
        exit(-1);
    }
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
#endif // NETWORK_DEBUG
        exit(-1);
    }
    if (recvResult == sf::Socket::Disconnected) {
#ifdef NETWORK_DEBUG
        cout << "Disconnected!" << endl;
#endif // NETWORK_DEBUG
        exit(-1);
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
        scene->AddPlayer(pk->id, { (float)pk->coord.x, (float)pk->coord.y }, pk->name);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_ADD_PLAYER 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_OBJECT_INFO:
    {
        sc_packet_object_info* pk = reinterpret_cast<sc_packet_object_info*>(buf);
        MainScene* scene = (MainScene*)g_gameFramework.GetScene();
        scene->Move(pk->id, { (float)pk->coord.x, (float)pk->coord.y });
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_OBJECT_INFO 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_CHAT:
    {
        sc_packet_chat* pk = reinterpret_cast<sc_packet_chat*>(buf);
        MainScene* scene = (MainScene*)g_gameFramework.GetScene();
        scene->SetChat(pk->id, pk->message);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_CHAT 수신" << endl;
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

#include "main.h"

#include "mainScene.h"

void InitInstance();

int main()
{
	InitInstance();

	g_window = make_shared<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Touhoukouyouden");
	
	while (g_window->isOpen()) {
        sf::Event event;
		while (g_window->pollEvent(event))
		{
			if (event.type == sf::Event::GainedFocus) {
				g_gameFramework.SetIsActive(true);
			}
			if (event.type == sf::Event::LostFocus) {
				g_gameFramework.SetIsActive(false);
			}
			if (event.type == sf::Event::TextEntered) {
				g_gameFramework.OnProcessingInputTextMessage(event);
			}
            if (event.type == sf::Event::MouseButtonPressed ||
                event.type == sf::Event::MouseButtonReleased ||
                event.type == sf::Event::MouseMoved) {
                g_gameFramework.OnProcessingMouseMessage(event, g_window);
            }
			if (event.type == sf::Event::Closed) {
				g_window->close();
			}
		}
		g_window->clear();
		g_gameFramework.FrameAdvance();
		g_window->display();
	}
}

void InitInstance()
{
    if (!g_font.loadFromFile("Resource\\NEXONLv1GothicLight.ttf")) {
        cout << "Font Loading Error!\n";
        exit(-1);
    }
	g_gameFramework.OnCreate();
}

void Send(void* packetBuf)
{
    unsigned short* packet = reinterpret_cast<unsigned short*>(packetBuf);
    size_t sent = 0;
    g_socket.send(packetBuf, packet[0], sent);
}
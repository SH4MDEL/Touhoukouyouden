#include "loginScene.h"

LoginScene::LoginScene()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = g_socket.connect("127.0.0.1", SERVER_PORT);
	g_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		exit(-1);
	}

	BuildObjects();
}

LoginScene::~LoginScene()
{
	DestroyObject();
}

void LoginScene::BuildObjects()
{
	m_buttonTexture = make_shared<sf::Texture>();
	m_buttonTexture->loadFromFile("Resource\\UI\\Button.png");

	m_gameStartUI = make_shared<ButtonUIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 0.8f, 0.5f });
	m_gameStartUI->SetPosition(sf::Vector2f{ 100, 0 });
	m_gameStartUI->SetSpriteTexture(m_buttonTexture, 0, 0, 278, 115);
	m_gameStartUI->SetText("Login");
	m_gameStartUI->SetTextFont(g_font);
	m_gameStartUI->SetTextColor(sf::Color(255, 255, 255));
	m_gameStartUI->SetClickEvent([&]() {
		cs_packet_login packet;
		packet.size = sizeof(cs_packet_login);
		packet.type = CS_PACKET_LOGIN;
		string player_name{ "PL" };
		player_name += to_string(GetCurrentProcessId());
		strcpy_s(packet.name, player_name.c_str());
		strcpy_s(packet.id, m_idBox->GetString().c_str());
		strcpy_s(packet.password, m_passwordBox->GetString().c_str());
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_LOGIN 송신" << endl;
#endif
	});

	m_idBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 100.f }, sf::Vector2f{ 2.f, 1.f }, 20);
	m_idBox->SetPosition(sf::Vector2f{ 200, 100 });
	m_idBox->SetSpriteTexture(m_buttonTexture, 0, 0, 278, 115);
	m_idBox->SetTextFont(g_font);
	m_idBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 200.f }, sf::Vector2f{ 2.f, 1.f }, 20);
	m_passwordBox->SetPosition(sf::Vector2f{ 200, 215 });
	m_passwordBox->SetSpriteTexture(m_buttonTexture, 0, 0, 278, 115);
	m_passwordBox->SetTextFont(g_font);
	m_passwordBox->SetTextColor(sf::Color(255, 255, 255));
}

void LoginScene::DestroyObject()
{

}

void LoginScene::Update(float timeElapsed)
{
	m_idBox->Update(timeElapsed);
	m_passwordBox->Update(timeElapsed);
}

void LoginScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	m_gameStartUI->Render(window);
	m_idBox->Render(window);
	m_passwordBox->Render(window);
}

void LoginScene::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	switch (inputEvent.type)
	{
	case sf::Event::TextEntered:
	{
		m_idBox->OnProcessingKeyboardMessage(inputEvent);
		m_passwordBox->OnProcessingKeyboardMessage(inputEvent);
		break;
	}
	}
}

void LoginScene::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	m_gameStartUI->OnProcessingMouseMessage(inputEvent, window);
	m_idBox->OnProcessingMouseMessage(inputEvent, window);
	m_passwordBox->OnProcessingMouseMessage(inputEvent, window);
}

void LoginScene::ProcessPacket(char* buf)
{
	switch (buf[1])
	{
	case SC_PACKET_LOGIN_CONFIRM:
	{
		sc_packet_login_confirm* pk = reinterpret_cast<sc_packet_login_confirm*>(buf);
		g_clientID = pk->id;
		g_gameFramework.ChangeScene(Tag::Main);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_LOGIN_CONFIRM 수신" << endl;
#endif
		break;
	case SC_PACKET_LOGIN_FAIL:
	{
		// 로그인 실패했으니까 다시 하라는 메시지 출력
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_LOGIN_FAIL 수신" << endl;
#endif
		break;
	}
	}
}

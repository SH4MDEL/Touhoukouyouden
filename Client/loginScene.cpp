#include "loginScene.h"

LoginScene::LoginScene()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = g_socket.connect("127.0.0.1", PORT_NUM);
	g_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"������ ������ �� �����ϴ�.\n";
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
	auto buttonTexture = make_shared<sf::Texture>();
	buttonTexture->loadFromFile("Resource\\UI\\Button.png");
	g_textures.insert({ "BUTTONTEXTURE", buttonTexture });

	m_gameStartUI = make_shared<ButtonUIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 0.8f, 0.5f });
	m_gameStartUI->SetPosition(sf::Vector2f{ 100, 0 });
	m_gameStartUI->SetSpriteTexture(g_textures["BUTTONTEXTURE"], 0, 0, 278, 115);
	m_gameStartUI->SetText("Login");
	m_gameStartUI->SetTextFont(g_font);
	m_gameStartUI->SetTextColor(sf::Color(255, 255, 255));
	m_gameStartUI->SetClickEvent([&]() {
		CS_LOGIN_PACKET packet;
		packet.size = sizeof(CS_LOGIN_PACKET);
		packet.type = CS_LOGIN;
		string player_name{ "PL" };
		player_name += to_string(GetCurrentProcessId());
		strcpy_s(packet.name, player_name.c_str());
		strcpy_s(packet.id, m_idBox->GetString().c_str());
		strcpy_s(packet.password, m_passwordBox->GetString().c_str());
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_LOGIN �۽�" << endl;
#endif
	});

	m_idBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 100.f }, sf::Vector2f{ 2.f, 1.f }, 20);
	m_idBox->SetPosition(sf::Vector2f{ 200, 100 });
	m_idBox->SetSpriteTexture(g_textures["BUTTONTEXTURE"], 0, 0, 278, 115);
	m_idBox->SetTextFont(g_font);
	m_idBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 200.f }, sf::Vector2f{ 2.f, 1.f }, 20);
	m_passwordBox->SetPosition(sf::Vector2f{ 200, 215 });
	m_passwordBox->SetSpriteTexture(g_textures["BUTTONTEXTURE"], 0, 0, 278, 115);
	m_passwordBox->SetTextFont(g_font);
	m_passwordBox->SetTextColor(sf::Color(255, 255, 255));
}

void LoginScene::DestroyObject()
{

}

void LoginScene::Update(float timeElapsed)
{
	if (g_clickEvent) {
		g_clickEvent();
		g_clickEvent = nullptr;
	}

	m_idBox->Update(timeElapsed);
	m_passwordBox->Update(timeElapsed);

	Recv();
}

void LoginScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	m_gameStartUI->Render(window);
	m_idBox->Render(window);
	m_passwordBox->Render(window);
}

void LoginScene::OnProcessingKeyboardMessage(float timeElapsed)
{
}

void LoginScene::OnProcessingInputTextMessage(sf::Event inputEvent)
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
	switch (buf[2])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* pk = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buf);
		g_clientID = pk->id;
		g_gameFramework.ChangeScene(Tag::Main);
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_INFO ����" << endl;
#endif
		break;
	}
	case SC_LOGIN_OK:
	{
		SC_LOGIN_OK_PACKET* pk = reinterpret_cast<SC_LOGIN_OK_PACKET*>(buf);
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_OK ����" << endl;
#endif
		break;
	}
	case SC_LOGIN_FAIL:
	{
		// �α��� ���������ϱ� �ٽ� �϶�� �޽��� ���
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_FAIL ����" << endl;
#endif
		break;
	}
	}
}

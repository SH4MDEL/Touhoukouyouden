#include "loginScene.h"

LoginScene::LoginScene()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = g_socket.connect("127.0.0.1", PORT_NUM);
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
	auto titleTexture = make_shared<sf::Texture>();
	titleTexture->loadFromFile("Resource\\UI\\TITLE.png");
	auto buttonTexture = make_shared<sf::Texture>();
	buttonTexture->loadFromFile("Resource\\UI\\SmallButton.png");
	auto textbarTexture = make_shared<sf::Texture>();
	textbarTexture->loadFromFile("Resource\\UI\\InputTextBar.png");
	g_textures.insert({ "TITLETEXTURE", titleTexture });
	g_textures.insert({ "BUTTONTEXTURE", buttonTexture });
	g_textures.insert({ "TEXTBARTEXTURE", textbarTexture });

	m_titleUI = make_shared<UIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_titleUI->SetSpriteTexture(g_textures["TITLETEXTURE"], 0, 0, 1440, 1040);

	m_loginUI = make_shared<ButtonUIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_loginUI->SetPosition(sf::Vector2f{ 950, 450 });
	m_loginUI->SetSpriteTexture(g_textures["BUTTONTEXTURE"], 0, 0, 121, 50);
	m_loginUI->SetText("Login");
	m_loginUI->SetTextFont(g_font);
	m_loginUI->SetTextColor(sf::Color(255, 255, 255));
	m_loginUI->SetTextSize(25);
	m_loginUI->SetClickEvent([&]() {
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
		cout << "CS_LOGIN 송신" << endl;
#endif
	});

	m_signupUI = make_shared<ButtonUIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_signupUI->SetPosition(sf::Vector2f{ 1190, 450 });
	m_signupUI->SetSpriteTexture(g_textures["BUTTONTEXTURE"], 0, 0, 121, 50);
	m_signupUI->SetText("Signup");
	m_signupUI->SetTextFont(g_font);
	m_signupUI->SetTextColor(sf::Color(255, 255, 255));
	m_signupUI->SetTextSize(25);
	m_signupUI->SetClickEvent([&]() {
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
		cout << "CS_LOGIN 송신" << endl;
#endif
		});

	m_idBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 950.f, 300.f }, sf::Vector2f{ 1.f, 1.f }, 20);
	m_idBox->SetSpriteTexture(g_textures["TEXTBARTEXTURE"], 0, 0, 361, 60);
	m_idBox->SetTextFont(g_font);
	m_idBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 950.f, 370.f }, sf::Vector2f{ 1.f, 1.f }, 20);
	m_passwordBox->SetSpriteTexture(g_textures["TEXTBARTEXTURE"], 0, 0, 361, 60);
	m_passwordBox->SetTextFont(g_font);
	m_passwordBox->SetTextColor(sf::Color(255, 255, 255));

	m_idMessageBox = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_idMessageBox->SetPosition(sf::Vector2f{ 890.f, 310.f });
	m_idMessageBox->SetText("ID:");
	m_idMessageBox->SetTextFont(g_font);
	m_idMessageBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordMessageBox = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_passwordMessageBox->SetPosition(sf::Vector2f{ 780.f, 380.f });
	m_passwordMessageBox->SetText("PASSWORD:");
	m_passwordMessageBox->SetTextFont(g_font);
	m_passwordMessageBox->SetTextColor(sf::Color(255, 255, 255));
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
	m_titleUI->Render(window);
	m_loginUI->Render(window);
	m_signupUI->Render(window);
	m_idBox->Render(window);
	m_passwordBox->Render(window);
	m_idMessageBox->Render(window);
	m_passwordMessageBox->Render(window);
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
	m_loginUI->OnProcessingMouseMessage(inputEvent, window);
	m_signupUI->OnProcessingMouseMessage(inputEvent, window);
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
		cout << "SC_LOGIN_INFO 수신" << endl;
#endif
		break;
	}
	case SC_LOGIN_OK:
	{
		SC_LOGIN_OK_PACKET* pk = reinterpret_cast<SC_LOGIN_OK_PACKET*>(buf);
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_OK 수신" << endl;
#endif
		break;
	}
	case SC_LOGIN_FAIL:
	{
		// 로그인 실패했으니까 다시 하라는 메시지 출력
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_FAIL 수신" << endl;
#endif
		break;
	}
	}
}

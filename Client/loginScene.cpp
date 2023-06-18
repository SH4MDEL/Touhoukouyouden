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
	auto noticeBoxTexture = make_shared<sf::Texture>();
	noticeBoxTexture->loadFromFile("Resource\\UI\\NoticeBox.png");
	auto cancelButtonTexture = make_shared<sf::Texture>();
	cancelButtonTexture->loadFromFile("Resource\\UI\\CancelButton.png");
	auto characterSelectBoxTexture = make_shared<sf::Texture>();
	characterSelectBoxTexture->loadFromFile("Resource\\UI\\CharacterSelectBox.png");
	auto reimuPortraitTexture = make_shared<sf::Texture>();
	reimuPortraitTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\PORTRAIT.png");
	auto youmuPortraitTexture = make_shared<sf::Texture>();
	youmuPortraitTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\PORTRAIT.png");
	auto patchouliPortraitTexture = make_shared<sf::Texture>();
	patchouliPortraitTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\PORTRAIT.png");
	g_textures.insert({ "TITLETEXTURE", titleTexture });
	g_textures.insert({ "BUTTONTEXTURE", buttonTexture });
	g_textures.insert({ "TEXTBARTEXTURE", textbarTexture });
	g_textures.insert({ "NOTICEBOXTEXTURE", noticeBoxTexture });
	g_textures.insert({ "CANCELBUTTONTEXTURE", cancelButtonTexture });
	g_textures.insert({ "CHARACTERSELECTBOXTEXTURE", characterSelectBoxTexture });
	g_textures.insert({ "REIMU_PORTRAIT", reimuPortraitTexture });
	g_textures.insert({ "YOUMU_PORTRAIT", youmuPortraitTexture });
	g_textures.insert({ "PATCHOULI_PORTRAIT", patchouliPortraitTexture });

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
		m_characterSelectBox->SetEnable();
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
	m_idMessageBox->SetPosition(sf::Vector2f{ 900.f, 330.f });
	m_idMessageBox->SetText("ID:");
	m_idMessageBox->SetTextFont(g_font);
	m_idMessageBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordMessageBox = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_passwordMessageBox->SetPosition(sf::Vector2f{ 850.f, 400.f });
	m_passwordMessageBox->SetText("PASSWORD:");
	m_passwordMessageBox->SetTextFont(g_font);
	m_passwordMessageBox->SetTextColor(sf::Color(255, 255, 255));

	m_noticeBox = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_noticeBox->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 - 210, WINDOW_HEIGHT / 2 - 160 });
	m_noticeBox->SetSpriteTexture(g_textures["NOTICEBOXTEXTURE"], 0, 0, 420, 321);
	m_noticeBox->SetText("");
	m_noticeBox->SetTextFont(g_font);
	m_noticeBox->SetTextColor(sf::Color(255, 255, 255));
	m_noticeBox->SetDisable();

	m_noticeCancelBox = make_shared<ButtonUIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_noticeCancelBox->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 + 160, WINDOW_HEIGHT / 2 - 170 });
	m_noticeCancelBox->SetSpriteTexture(g_textures["CANCELBUTTONTEXTURE"], 0, 0, 60, 60);
	m_noticeCancelBox->SetClickEvent([&]() {
		m_noticeBox->SetDisable();
	});
	m_noticeBox->SetChild(m_noticeCancelBox);

	m_characterSelectBox = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_characterSelectBox->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 - 487, WINDOW_HEIGHT / 2 - 300 });
	m_characterSelectBox->SetSpriteTexture(g_textures["CHARACTERSELECTBOXTEXTURE"], 0, 0, 974, 600);
	m_characterSelectBox->SetDisable();

	m_characterSelectCancelBox = make_shared<ButtonUIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_characterSelectCancelBox->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 + 437, WINDOW_HEIGHT / 2 - 310 });
	m_characterSelectCancelBox->SetSpriteTexture(g_textures["CANCELBUTTONTEXTURE"], 0, 0, 60, 60);
	m_characterSelectCancelBox->SetClickEvent([&]() {
		m_characterSelectBox->SetDisable();
		});
	m_characterSelectBox->SetChild(m_characterSelectCancelBox);

	m_reimuButton = make_shared<ButtonUIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_reimuButton->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 - 455, WINDOW_HEIGHT / 2 - 150 });
	m_reimuButton->SetSpriteTexture(g_textures["REIMU_PORTRAIT"], 0, 0, 320, 300);
	m_reimuButton->SetClickEvent([&]() {
		CS_SIGNUP_PACKET packet;
		packet.size = sizeof(CS_SIGNUP_PACKET);
		packet.type = CS_SIGNUP;
		packet.serial = Serial::Character::HAKUREI_REIMU;
		strcpy_s(packet.id, m_idBox->GetString().c_str());
		strcpy_s(packet.password, m_passwordBox->GetString().c_str());
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_SIGNUP 송신" << endl;
#endif
		});
	m_characterSelectBox->SetChild(m_reimuButton);

	m_youmuButton = make_shared<ButtonUIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_youmuButton->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 - 147, WINDOW_HEIGHT / 2 - 150 });
	m_youmuButton->SetSpriteTexture(g_textures["YOUMU_PORTRAIT"], 0, 0, 308, 300);
	m_youmuButton->SetClickEvent([&]() {
		CS_SIGNUP_PACKET packet;
		packet.size = sizeof(CS_SIGNUP_PACKET);
		packet.type = CS_SIGNUP;
		packet.serial = Serial::Character::KONPAKU_YOUMU;
		strcpy_s(packet.id, m_idBox->GetString().c_str());
		strcpy_s(packet.password, m_passwordBox->GetString().c_str());
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_SIGNUP 송신" << endl;
#endif
		});
	m_characterSelectBox->SetChild(m_youmuButton);

	m_patchouliButton = make_shared<ButtonUIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_patchouliButton->SetPosition(sf::Vector2f{ WINDOW_WIDTH / 2 + 172, WINDOW_HEIGHT / 2 - 150 });
	m_patchouliButton->SetSpriteTexture(g_textures["PATCHOULI_PORTRAIT"], 0, 0, 306, 300);
	m_patchouliButton->SetClickEvent([&]() {
		CS_SIGNUP_PACKET packet;
		packet.size = sizeof(CS_SIGNUP_PACKET);
		packet.type = CS_SIGNUP;
		packet.serial = Serial::Character::PATCHOULI_KNOWLEDGE;
		strcpy_s(packet.id, m_idBox->GetString().c_str());
		strcpy_s(packet.password, m_passwordBox->GetString().c_str());
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_SIGNUP 송신" << endl;
#endif
		});
	m_characterSelectBox->SetChild(m_patchouliButton);
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
	m_characterSelectBox->Render(window);
	m_noticeBox->Render(window);
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
	m_characterSelectBox->OnProcessingMouseMessage(inputEvent, window);
	m_noticeBox->OnProcessingMouseMessage(inputEvent, window);
}

void LoginScene::ProcessPacket(char* buf)
{
	switch (buf[2])
	{
	case SC_LOGIN_OK:
	{
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_OK 수신" << endl;
#endif
		g_gameFramework.ChangeScene(Tag::Main);
		break;
	}
	case SC_LOGIN_FAIL:
	{
		m_noticeBox->SetText("Login Fail..");
		m_noticeBox->SetEnable();
		// 로그인 실패했으니까 다시 하라는 메시지 출력
#ifdef NETWORK_DEBUG
		cout << "SC_LOGIN_FAIL 수신" << endl;
#endif
		break;
	}
	case SC_SIGNUP_OK:
	{
		m_noticeBox->SetText("New Character Created!!");
		m_noticeBox->SetEnable();
#ifdef NETWORK_DEBUG
		cout << "SC_SIGNUP_OK 수신" << endl;
#endif
		break;
	}
	case SC_SIGNUP_FAIL:
	{
		m_noticeBox->SetText("Sign Up Fail..");
		m_noticeBox->SetEnable();
#ifdef NETWORK_DEBUG
		cout << "SC_SIGNUP_FAIL 수신" << endl;
#endif
		break;
	}
	}
}

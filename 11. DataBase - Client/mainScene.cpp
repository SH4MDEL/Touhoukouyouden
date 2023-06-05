#include "mainScene.h"

MainScene::MainScene()
{
	BuildObjects();
}

MainScene::~MainScene()
{
	DestroyObject();
}

void MainScene::BuildObjects()
{
	m_boardTexture = make_shared<sf::Texture>();
	m_boardTexture->loadFromFile("..\\Resource\\Chessboard.png");

	m_pieceTexture = make_shared<sf::Texture>();
	m_pieceTexture->loadFromFile("..\\Resource\\Piece.png");

	m_whiteTile = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_whiteTile->SetSpriteTexture(m_boardTexture, 0, 0, TILE_WIDTH, TILE_WIDTH);
	m_blackTile = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_blackTile->SetSpriteTexture(m_boardTexture, 129, 0, TILE_WIDTH, TILE_WIDTH);
}

void MainScene::DestroyObject()
{
	m_avatar.reset();
	for (auto& player : m_players) {
		player.second.reset();
	}
	m_players.clear();
}

void MainScene::Update(float timeElapsed)
{
	Recv();
	if (m_avatar) m_avatar->Update(timeElapsed);
	for (auto& player : m_players) player.second->Update(timeElapsed);
}

void MainScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	for (int i = 0; i < SCREEN_WIDTH; ++i) {
		for (int j = 0; j < SCREEN_HEIGHT; ++j) {
			int tileX = i + g_leftX;
			int tileY = j + g_topY;
			if (tileX < 0 || tileX > MAP_WIDTH || tileY < 0 || tileY > MAP_HEIGHT) continue;
			if (0 == (tileX / 3 + tileY / 3) % 2) {
				m_whiteTile->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_whiteTile->Render(window);
			}
			else
			{
				m_blackTile->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_blackTile->Render(window);
			}
		}
	}
	if (m_avatar) m_avatar->Render(window);
	for (auto& player : m_players) player.second->Render(window);
	
	if (m_avatar) {
		sf::Text text;
		text.setFont(g_font);
		char buf[100];
		sprintf_s(buf, "(%d, %d)", (int)m_avatar->GetPosition().x, (int)m_avatar->GetPosition().y);
		text.setFillColor(sf::Color::Yellow);
		text.setString(buf);
		g_window->draw(text);
	}
}

void MainScene::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	if (inputEvent.type == sf::Event::KeyPressed) {
		switch (inputEvent.key.code)
		{
		case sf::Keyboard::Left:
		case sf::Keyboard::Right:
		case sf::Keyboard::Up:
		case sf::Keyboard::Down:
			cs_packet_move packet;
			packet.size = sizeof(cs_packet_move);
			packet.type = CS_PACKET_MOVE;
			packet.direction = sf::Keyboard::Down - inputEvent.key.code;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_PACKET_MOVE 송신" << endl;
#endif
			break;

		}
	}
}

void MainScene::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
}

void MainScene::AddPlayer(int id, sf::Vector2f position, const char* name)
{
	if (id == g_clientID) {
		m_avatar = make_shared<Piece>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		m_avatar->SetSpriteTexture(m_pieceTexture, 0, 0, 64, 64);
		m_avatar->SetPosition(position);
		m_avatar->SetName(name);
		g_leftX = (int)position.x - 7; g_topY = (int)position.y - 7;
	}
	else {
		m_players[id] = make_shared<Piece>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		m_players[id]->SetSpriteTexture(m_pieceTexture, 0, 0, 64, 64);
		m_players[id]->SetPosition(position);
		m_players[id]->SetName(name);
	}
}

void MainScene::ExitPlayer(INT id)
{
	m_players.erase(id);
}

void MainScene::Move(INT id, sf::Vector2f position)
{
	if (id == g_clientID) {
		m_avatar->SetPosition(position);
		g_leftX = position.x - 7; g_topY = position.y - 7;
	}
	else {
		m_players[id]->SetPosition(position);
	}
}

void MainScene::SetChat(INT id, const char* chat)
{
	if (id == g_clientID) {
		m_avatar->SetChat(chat);
	}
	else {
		m_players[id]->SetChat(chat);
	}
}

void MainScene::ProcessPacket(char* buf)
{
	switch (buf[1])
	{
	case SC_PACKET_ADD_PLAYER:
	{
		sc_packet_add_player* pk = reinterpret_cast<sc_packet_add_player*>(buf);
		sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y};
		AddPlayer(pk->id, pos, pk->name);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_ADD_PLAYER 수신" << endl;
#endif
		break;
	}
	case SC_PACKET_OBJECT_INFO:
	{
		sc_packet_object_info* pk = reinterpret_cast<sc_packet_object_info*>(buf);
		sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y };
		Move(pk->id, pos);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_OBJECT_INFO 수신" << endl;
#endif
		break;
	}
	case SC_PACKET_CHAT:
	{
		sc_packet_chat* pk = reinterpret_cast<sc_packet_chat*>(buf);
		SetChat(pk->id, pk->message);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_CHAT 수신" << endl;
#endif
		break;
	}
	case SC_PACKET_EXIT_PLAYER:
	{
		sc_packet_exit_player* pk = reinterpret_cast<sc_packet_exit_player*>(buf);
		ExitPlayer(pk->id);
#ifdef NETWORK_DEBUG
		cout << "SC_PACKET_EXIT_PLAYER 수신" << endl;
#endif
		break;
	}
	}
}
#include "mainScene.h"

MainScene::MainScene(Tag tag) : Scene(tag)
{
}

void MainScene::OnCreate()
{
	BuildObjects();
}

void MainScene::BuildObjects()
{
	m_boardTexture = make_shared<sf::Texture>();
	m_boardTexture->loadFromFile("..\\Resource\\Chessboard.png");

	m_pieceTexture = make_shared<sf::Texture>();
	m_pieceTexture->loadFromFile("..\\Resource\\Piece.png");

	m_whiteTile = make_shared<Object>();
	m_whiteTile->SetTexture(m_boardTexture, 0, 0, TILE_WIDTH, TILE_WIDTH);
	m_blackTile = make_shared<Object>();
	m_blackTile->SetTexture(m_boardTexture, 129, 0, TILE_WIDTH, TILE_WIDTH);
}


void MainScene::OnDestroy()
{
	DestroyObject();
}

void MainScene::DestroyObject()
{
	m_players.clear();
}

void MainScene::Update(float timeElapsed)
{
	Recv();
}

void MainScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	for (int i = 0; i < SCREEN_WIDTH; ++i) {
		for (int j = 0; j < SCREEN_HEIGHT; ++j) {
			int tileX = i + g_leftX;
			int tileY = j + g_topY;
			if (tileX < 0 || tileX > MAP_WIDTH || tileY < 0 || tileY > MAP_HEIGHT) continue;
			if (0 == (tileX / 3 + tileY / 3) % 2) {
				m_whiteTile->SetPosition({ (short)(TILE_WIDTH * i), (short)(TILE_WIDTH * j) });
				m_whiteTile->Render(window);
			}
			else
			{
				m_blackTile->SetPosition({ (short)(TILE_WIDTH * i), (short)(TILE_WIDTH * j) });
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
		sprintf_s(buf, "(%d, %d)", m_avatar->GetPosition().x, m_avatar->GetPosition().y);
		text.setFillColor(sf::Color::Black);
		text.setString(buf);
		g_window->draw(text);
	}
}

void MainScene::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	switch (inputEvent.key.code)
	{
	case sf::Keyboard::Left:
	case sf::Keyboard::Right:
	case sf::Keyboard::Up:
	case sf::Keyboard::Down:
		cs_packet_move packet;
		packet.size = sizeof(cs_packet_move);
		packet.type = CS_PACKET_MOVE;
		packet.id = g_clientID;
		packet.direction = sf::Keyboard::Down - inputEvent.key.code;
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_MOVE ¼Û½Å" << endl;
#endif
		break;
	}
}

void MainScene::AddPlayer(int id, Short2 position)
{
	if (id == g_clientID) {
		m_avatar = make_shared<Piece>();
		m_avatar->SetTexture(m_pieceTexture, 0, 0, 64, 64);
		m_avatar->SetPosition(position);
		g_leftX = position.x - 4; g_topY = position.y - 4;
	}
	else {
		m_players[id] = make_shared<Piece>();
		m_players[id]->SetTexture(m_pieceTexture, 0, 0, 64, 64);
		m_players[id]->SetPosition(position);
	}
}

void MainScene::ExitPlayer(INT id)
{
	m_players.erase(id);
}

void MainScene::Move(INT id, Short2 position)
{
	if (id == g_clientID) {
		m_avatar->SetPosition(position);
		g_leftX = position.x - 4; g_topY = position.y - 4;
	}
	else {
		m_players[id]->SetPosition(position);
	}
}

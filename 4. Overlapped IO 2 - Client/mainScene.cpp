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
			if (tileX < 0 || tileY < 0) continue;
			if (0 == (tileX / 3 + tileY / 3) % 2) {
				m_whiteTile->SetPosition({ TILE_WIDTH * i, TILE_WIDTH * j });
				m_whiteTile->Render(window);
			}
			else
			{
				m_blackTile->SetPosition({ TILE_WIDTH * i, TILE_WIDTH * j });
				m_blackTile->Render(window);
			}
		}
	}
	if (m_avatar) m_avatar->Render(window);
	for (auto& player : m_players) player.second->Render(window);
	
	//sf::Text text;
	//text.setFont(g_font);
	//char buf[100];
	////sprintf_s(buf, "(%d, %d)", m_avatar.m_x, avatar.m_y);
	//text.setString(buf);
	//g_window->draw(text);
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
		packet.coord.x = Move::dx[sf::Keyboard::Down - inputEvent.key.code];
		packet.coord.y = Move::dy[sf::Keyboard::Down - inputEvent.key.code];
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_PACKET_MOVE ¼Û½Å" << endl;
#endif
		break;
	default:
		break;
	}
}

void MainScene::AddPlayer(int id, POINT position)
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

void MainScene::Move(INT id, POINT position)
{
	if (id == g_clientID) {
		m_avatar->SetPosition(position);
		g_leftX = position.x - 4; g_topY = position.y - 4;
	}
	else {
		m_players[id]->SetPosition(position);
	}
}

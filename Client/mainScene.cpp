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
	auto mapTexture = make_shared<sf::Texture>();
	mapTexture->loadFromFile("Resource\\Chessboard.png");

	g_textures.insert({ "MAP", mapTexture });

	auto reimuIdleTexture = make_shared<sf::Texture>();
	reimuIdleTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\IDLE.png");
	auto reimuWalkTexture = make_shared<sf::Texture>();
	reimuWalkTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\WALK.png");
	auto reimuAttackTexture = make_shared<sf::Texture>();
	reimuAttackTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\ATTACK.png");
	auto reimuDieTexture = make_shared<sf::Texture>();
	reimuDieTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\DIE.png");

	g_textures.insert({ "REIMU_IDLE", reimuIdleTexture });
	g_textures.insert({ "REIMU_WALK", reimuWalkTexture });
	g_textures.insert({ "REIMU_ATTACK", reimuAttackTexture });
	g_textures.insert({ "REIMU_DIE", reimuDieTexture });

	auto youmuIdleTexture = make_shared<sf::Texture>();
	youmuIdleTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\IDLE.png");
	auto youmuWalkTexture = make_shared<sf::Texture>();
	youmuWalkTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\WALK.png");
	auto youmuAttackTexture = make_shared<sf::Texture>();
	youmuAttackTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\ATTACK.png");
	auto youmuDieTexture = make_shared<sf::Texture>();
	youmuDieTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\DIE.png");

	g_textures.insert({ "YOUMU_IDLE", youmuIdleTexture });
	g_textures.insert({ "YOUMU_WALK", youmuWalkTexture });
	g_textures.insert({ "YOUMU_ATTACK", youmuAttackTexture });
	g_textures.insert({ "YOUMU_DIE", youmuDieTexture });

	auto patchouliIdleTexture = make_shared<sf::Texture>();
	patchouliIdleTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\IDLE.png");
	auto patchouliWalkTexture = make_shared<sf::Texture>();
	patchouliWalkTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\WALK.png");
	auto patchouliAttackTexture = make_shared<sf::Texture>();
	patchouliAttackTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\ATTACK.png");
	auto patchouliDieTexture = make_shared<sf::Texture>();
	patchouliDieTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\DIE.png");

	g_textures.insert({ "PATCHOULI_IDLE", patchouliIdleTexture });
	g_textures.insert({ "PATCHOULI_WALK", patchouliWalkTexture });
	g_textures.insert({ "PATCHOULI_ATTACK", patchouliAttackTexture });
	g_textures.insert({ "PATCHOULI_DIE", patchouliDieTexture });

	m_whiteTile = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_whiteTile->SetSpriteTexture(g_textures["MAP"], 0, 0, TILE_WIDTH, TILE_WIDTH);
	m_blackTile = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_blackTile->SetSpriteTexture(g_textures["MAP"], 129, 0, TILE_WIDTH, TILE_WIDTH);
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
			if (tileX < 0 || tileX > W_WIDTH || tileY < 0 || tileY > W_HEIGHT) continue;
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
			packet.type = CS_MOVE;
			packet.direction = sf::Keyboard::Down - inputEvent.key.code;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_MOVE 송신" << endl;
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
		m_avatar = make_shared<Player>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		SetAnimationInfo(CharacterInfo::HAKUREI_REIMU, m_avatar);
		m_avatar->SetPosition(position);
		m_avatar->SetName(name);
		g_leftX = (int)position.x - 7; g_topY = (int)position.y - 7;
	}
	else {
		m_players[id] = make_shared<Player>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		SetAnimationInfo(CharacterInfo::PATCHOULI_KNOWLEDGE, m_players[id]);
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

void MainScene::SetAnimationInfo(CharacterInfo characterInfo, const shared_ptr<AnimationObject>& object)
{
	switch (characterInfo)
	{
	case HAKUREI_REIMU:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["REIMU_IDLE"], sf::IntRect{0, 0, 94, 102},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["REIMU_WALK"], sf::IntRect{0, 0, 110, 105},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["REIMU_ATTACK"], sf::IntRect{0, 0, 102, 100},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["REIMU_DIE"], sf::IntRect{0, 0, 104, 102},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		break;
	case KONPAKU_YOUMU:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["YOUMU_IDLE"], sf::IntRect{0, 0, 108, 82},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["YOUMU_WALK"], sf::IntRect{0, 0, 100, 82},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["YOUMU_ATTACK"], sf::IntRect{0, 0, 122, 84},
			sf::Vector2i{13, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["YOUMU_DIE"], sf::IntRect{0, 0, 110, 110},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		break;
	case PATCHOULI_KNOWLEDGE:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["PATCHOULI_IDLE"], sf::IntRect{0, 0, 56, 104},
			sf::Vector2i{18, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["PATCHOULI_WALK"], sf::IntRect{0, 0, 46, 99},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["PATCHOULI_ATTACK"], sf::IntRect{0, 0, 74, 106},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["PATCHOULI_DIE"], sf::IntRect{0, 0, 106, 82},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		break;
	}
}

void MainScene::ProcessPacket(char* buf)
{
	switch (buf[1])
	{
	case SC_ADD_OBJECT:
	{
		sc_packet_add_player* pk = reinterpret_cast<sc_packet_add_player*>(buf);
		sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y};
		AddPlayer(pk->id, pos, pk->name);
#ifdef NETWORK_DEBUG
		cout << "SC_ADD_OBJECT 수신" << endl;
#endif
		break;
	}
	case SC_MOVE_OBJECT:
	{
		sc_packet_object_info* pk = reinterpret_cast<sc_packet_object_info*>(buf);
		sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y };
		Move(pk->id, pos);
#ifdef NETWORK_DEBUG
		cout << "SC_MOVE_OBJECT 수신" << endl;
#endif
		break;
	}
	case SC_CHAT:
	{
		sc_packet_chat* pk = reinterpret_cast<sc_packet_chat*>(buf);
		SetChat(pk->id, pk->message);
#ifdef NETWORK_DEBUG
		cout << "SC_CHAT 수신" << endl;
#endif
		break;
	}
	case SC_REMOVE_OBJECT:
	{
		sc_packet_exit_player* pk = reinterpret_cast<sc_packet_exit_player*>(buf);
		ExitPlayer(pk->id);
#ifdef NETWORK_DEBUG
		cout << "SC_REMOVE_OBJECT 수신" << endl;
#endif
		break;
	}
	}
}
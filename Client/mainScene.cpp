#include "mainScene.h"

MainScene::MainScene() : m_inputState {false}, m_chatTime{0.f}
{
	BuildObjects();
}

MainScene::~MainScene()
{
	DestroyObject();
}

void MainScene::BuildObjects()
{
	ifstream in("map.txt", ios::binary);

	for (int i = 0; i < W_HEIGHT; ++i) {
		for (int j = 0; j < W_WIDTH; ++j) {
			in >> m_map[i][j];
		}
	}

	m_hpUI = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_hpUI->SetPosition(sf::Vector2f{ 1230.f, 100.f });
	m_hpUI->SetText("");
	m_hpUI->SetTextFont(g_font);
	m_hpUI->SetTextColor(sf::Color(255, 255, 255));

	m_levelUI = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_levelUI->SetPosition(sf::Vector2f{ 1230.f, 150.f });
	m_levelUI->SetText("");
	m_levelUI->SetTextFont(g_font);
	m_levelUI->SetTextColor(sf::Color(255, 255, 255));

	m_expUI = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
	m_expUI->SetPosition(sf::Vector2f{ 1230.f, 200.f });
	m_expUI->SetText("");
	m_expUI->SetTextFont(g_font);
	m_expUI->SetTextColor(sf::Color(255, 255, 255));

	for (int i = 0; auto & message : m_message) {
		message = make_shared<UIObject>(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
		message->SetPosition(sf::Vector2f{ 1230.f, 800 - (float)i++ * 20.f});
		message->SetTextSize(15);
		message->SetText("");
		message->SetTextFont(g_font);
		message->SetTextColor(sf::Color(255, 255, 255));
	}

	auto textbarTexture = make_shared<sf::Texture>();
	textbarTexture->loadFromFile("Resource\\UI\\InputTextBar.png");
	g_textures.insert({ "TEXTBARTEXTURE", textbarTexture });

	m_messageBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 1060.f, 850.f }, sf::Vector2f{ 1.f, 1.f }, 20);
	m_messageBox->SetSpriteTexture(g_textures["TEXTBARTEXTURE"], 0, 0, 361, 60);
	m_messageBox->SetTextFont(g_font);
	m_messageBox->SetTextColor(sf::Color(255, 255, 255));

	auto mapTexture = make_shared<sf::Texture>();
	mapTexture->loadFromFile("Resource\\Chessboard.png");
	auto henesysBlock = make_shared<sf::Texture>();
	henesysBlock->loadFromFile("Resource\\TILE\\HENESYS_BLOCKING.png");
	auto henesysNonblock = make_shared<sf::Texture>();
	henesysNonblock->loadFromFile("Resource\\TILE\\HENESYS_NONBLOCKING.png");
	g_textures.insert({ "MAP", mapTexture });
	g_textures.insert({ "HENESYS_BLOCKING", henesysBlock });
	g_textures.insert({ "HENESYS_NONBLOCKING", henesysNonblock });

	auto reimuIdleTexture = make_shared<sf::Texture>();
	reimuIdleTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\IDLE.png");
	auto reimuWalkTexture = make_shared<sf::Texture>();
	reimuWalkTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\WALK.png");
	auto reimuAttackTexture = make_shared<sf::Texture>();
	reimuAttackTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\ATTACK.png");
	auto reimuSkillTexture = make_shared<sf::Texture>();
	reimuSkillTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\SKILL.png");
	auto reimuDieTexture = make_shared<sf::Texture>();
	reimuDieTexture->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\DIE.png");

	g_textures.insert({ "REIMU_IDLE", reimuIdleTexture });
	g_textures.insert({ "REIMU_WALK", reimuWalkTexture });
	g_textures.insert({ "REIMU_ATTACK", reimuAttackTexture });
	g_textures.insert({ "REIMU_SKILL", reimuSkillTexture });
	g_textures.insert({ "REIMU_DIE", reimuDieTexture });

	auto youmuIdleTexture = make_shared<sf::Texture>();
	youmuIdleTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\IDLE.png");
	auto youmuWalkTexture = make_shared<sf::Texture>();
	youmuWalkTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\WALK.png");
	auto youmuAttackTexture = make_shared<sf::Texture>();
	youmuAttackTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\ATTACK.png");
	auto youmuSkillTexture = make_shared<sf::Texture>();
	youmuSkillTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\SKILL.png");
	auto youmuDieTexture = make_shared<sf::Texture>();
	youmuDieTexture->loadFromFile("Resource\\CHARACTER\\KONPAKU_YOUMU\\DIE.png");

	g_textures.insert({ "YOUMU_IDLE", youmuIdleTexture });
	g_textures.insert({ "YOUMU_WALK", youmuWalkTexture });
	g_textures.insert({ "YOUMU_ATTACK", youmuAttackTexture });
	g_textures.insert({ "YOUMU_SKILL", youmuSkillTexture });
	g_textures.insert({ "YOUMU_DIE", youmuDieTexture });

	auto patchouliIdleTexture = make_shared<sf::Texture>();
	patchouliIdleTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\IDLE.png");
	auto patchouliWalkTexture = make_shared<sf::Texture>();
	patchouliWalkTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\WALK.png");
	auto patchouliAttackTexture = make_shared<sf::Texture>();
	patchouliAttackTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\ATTACK.png");
	auto patchouliSkillTexture = make_shared<sf::Texture>();
	patchouliSkillTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\SKILL.png");
	auto patchouliDieTexture = make_shared<sf::Texture>();
	patchouliDieTexture->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\DIE.png");

	g_textures.insert({ "PATCHOULI_IDLE", patchouliIdleTexture });
	g_textures.insert({ "PATCHOULI_WALK", patchouliWalkTexture });
	g_textures.insert({ "PATCHOULI_ATTACK", patchouliAttackTexture });
	g_textures.insert({ "PATCHOULI_SKILL", patchouliSkillTexture });
	g_textures.insert({ "PATCHOULI_DIE", patchouliDieTexture });

	auto shroomIdleTexture = make_shared<sf::Texture>();
	shroomIdleTexture->loadFromFile("Resource\\MONSTER\\SHROOM\\IDLE.png");
	auto shroomWalkTexture = make_shared<sf::Texture>();
	shroomWalkTexture->loadFromFile("Resource\\MONSTER\\SHROOM\\WALK.png");
	auto shroomDieTexture = make_shared<sf::Texture>();
	shroomDieTexture->loadFromFile("Resource\\MONSTER\\SHROOM\\DIE.png");

	g_textures.insert({ "SHROOM_IDLE", shroomIdleTexture });
	g_textures.insert({ "SHROOM_WALK", shroomWalkTexture });
	g_textures.insert({ "SHROOM_DIE", shroomDieTexture });

	auto mushroomIdleTexture = make_shared<sf::Texture>();
	mushroomIdleTexture->loadFromFile("Resource\\MONSTER\\MUSHROOM\\IDLE.png");
	auto mushroomWalkTexture = make_shared<sf::Texture>();
	mushroomWalkTexture->loadFromFile("Resource\\MONSTER\\MUSHROOM\\WALK.png");
	auto mushroomDieTexture = make_shared<sf::Texture>();
	mushroomDieTexture->loadFromFile("Resource\\MONSTER\\MUSHROOM\\DIE.png");

	g_textures.insert({ "MUSHROOM_IDLE", mushroomIdleTexture });
	g_textures.insert({ "MUSHROOM_WALK", mushroomWalkTexture });
	g_textures.insert({ "MUSHROOM_DIE", mushroomDieTexture });

	auto ribbonpigIdleTexture = make_shared<sf::Texture>();
	ribbonpigIdleTexture->loadFromFile("Resource\\MONSTER\\RIBBONPIG\\IDLE.png");
	auto ribbonpigWalkTexture = make_shared<sf::Texture>();
	ribbonpigWalkTexture->loadFromFile("Resource\\MONSTER\\RIBBONPIG\\WALK.png");
	auto ribbonpigDieTexture = make_shared<sf::Texture>();
	ribbonpigDieTexture->loadFromFile("Resource\\MONSTER\\RIBBONPIG\\DIE.png");

	g_textures.insert({ "RIBBONPIG_IDLE", ribbonpigIdleTexture });
	g_textures.insert({ "RIBBONPIG_WALK", ribbonpigWalkTexture });
	g_textures.insert({ "RIBBONPIG_DIE", ribbonpigDieTexture });

	auto reimuEffect = make_shared<sf::Texture>();
	reimuEffect->loadFromFile("Resource\\CHARACTER\\HAKUREI_REIMU\\EFFECT.png");
	auto patchouliEffect = make_shared<sf::Texture>();
	patchouliEffect->loadFromFile("Resource\\CHARACTER\\PATCHOULI_KNOWLEDGE\\EFFECT.png");

	g_textures.insert({ "REIMU_EFFECT", reimuEffect });
	g_textures.insert({ "PATCHOULI_EFFECT", patchouliEffect });

	m_block = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_block->SetSpriteTexture(g_textures["MAP"], 0, 0, TILE_WIDTH, TILE_WIDTH);
	m_nonblock = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_nonblock->SetSpriteTexture(g_textures["MAP"], 129, 0, TILE_WIDTH, TILE_WIDTH);
	m_henesysBlock = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_henesysBlock->SetSpriteTexture(g_textures["HENESYS_BLOCKING"], 0, 0, TILE_WIDTH, TILE_WIDTH);
	m_henesysNonblock = make_shared<Object>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
	m_henesysNonblock->SetSpriteTexture(g_textures["HENESYS_NONBLOCKING"], 0, 0, TILE_WIDTH, TILE_WIDTH);
}

void MainScene::DestroyObject()
{
	m_avatar.reset();
	for (auto& player : m_players) {
		player.second.reset();
	}
	m_players.clear();
}

void MainScene::SetMessage(const char* message)
{
	for (int i = 19; i > 0; --i) {
		m_message[i]->SetText(m_message[i - 1]->GetString().c_str());
	}
	m_message[0]->SetText(message);
}

void MainScene::Update(float timeElapsed)
{
	Recv();
	if (m_avatar) m_avatar->Update(timeElapsed);
	for (auto& player : m_players) player.second->Update(timeElapsed);
	for (auto& effect : m_effects) effect->Update(timeElapsed);
	for (auto& itr = m_effects.begin(); itr != m_effects.end(); ++itr) {
		if ((*itr)->IsFinish()) {
			m_effects.erase(itr);
			break;
		}
	}

	m_messageBox->Update(timeElapsed);

	if (m_chatTime > 0) {
		m_chatTime -= timeElapsed;
	}
}

void MainScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	for (int i = 0; i < SCREEN_WIDTH; ++i) {
		for (int j = 0; j < SCREEN_HEIGHT; ++j) {
			int tileX = i + g_leftX;
			int tileY = j + g_topY;
			if (tileX < 0 || tileX > W_WIDTH || tileY < 0 || tileY > W_HEIGHT) continue;
			if (m_map[tileY][tileX] == TileInfo::UNDEFINED_NONBLOCK) {
				m_nonblock->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_nonblock->Render(window);
			}
			else if (m_map[tileY][tileX] == TileInfo::UNDEFINED_BLOCK) {
				m_block->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_block->Render(window);
			}
			else if (m_map[tileY][tileX] == TileInfo::HENESYS_NONBLOCK) {
				m_henesysNonblock->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_henesysNonblock->Render(window);
			}
			else if (m_map[tileY][tileX] == TileInfo::HENESYS_BLOCK) {
				m_henesysBlock->SetPosition({ (float)(TILE_WIDTH * i), (float)(TILE_WIDTH * j) });
				m_henesysBlock->Render(window);
			}
		}
	}
	if (m_avatar) m_avatar->Render(window);
	for (auto& player : m_players) player.second->Render(window);
	for (auto& effect : m_effects) effect->Render(window);
	
	if (m_avatar) {
		sf::Text text;
		text.setFont(g_font);
		char buf[100];
		sprintf_s(buf, "(%d, %d)", (int)m_avatar->GetPosition().x, (int)m_avatar->GetPosition().y);
		text.setFillColor(sf::Color::Yellow);
		text.setString(buf);
		g_window->draw(text);
	}

	if (m_hpUI) m_hpUI->Render(window);
	if (m_levelUI) m_levelUI->Render(window);
	if (m_expUI) m_expUI->Render(window);

	for (auto & message : m_message) {
		message->Render(window);
	}
	if (m_messageBox) m_messageBox->Render(window);
}

void MainScene::OnProcessingKeyboardMessage(float timeElapsed)
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		if (m_inputState) {
			m_inputState = false;
			m_messageBox->SetType(ButtonUIObject::Type::NOACTIVE);
			if (m_messageBox->GetString().size() && m_chatTime <= 0.f) {
				m_chatTime = m_chatCoolTime;
				CS_CHAT_PACKET packet;
				packet.size = sizeof(PACKET) + m_messageBox->GetString().size() + 1;
				packet.type = CS_CHAT;
				strcpy_s(packet.mess, m_messageBox->GetString().c_str());
				Send(&packet);
#ifdef NETWORK_DEBUG
				cout << "CS_CHAT 송신" << endl;
#endif
			}
		}
		else {
			m_inputState = true;
			m_messageBox->SetType(ButtonUIObject::Type::ACTIVE);
		}
	}
	if (!m_inputState) {
		if (m_avatar) m_avatar->OnProcessingKeyboardMessage(timeElapsed);
	}
}

void MainScene::OnProcessingInputTextMessage(sf::Event inputEvent)
{
	switch (inputEvent.type)
	{
	case sf::Event::TextEntered:
	{
		if (m_inputState) m_messageBox->OnProcessingKeyboardMessage(inputEvent);
		break;
	}
	}
}

void MainScene::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
}

void MainScene::AddPlayer(int id, int serial, sf::Vector2f position, const char* name)
{
	if (id == g_clientID) {
		m_avatar = make_shared<Player>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		SetAnimationInfo(serial, m_avatar);
		m_avatar->SetPosition(position);
		m_avatar->SetName(name);
		m_avatar->SetSerial(serial);
		g_leftX = (int)position.x - 7; g_topY = (int)position.y - 7;
	}
	else {
		m_players[id] = make_shared<Player>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 1.f, 1.f });
		SetAnimationInfo(serial, m_players[id]);
		m_players[id]->SetPosition(position);
		m_players[id]->SetName(name);
	}
}

void MainScene::ExitPlayer(int id)
{
	m_players.erase(id);
}

void MainScene::Move(INT id, sf::Vector2f position)
{
	if (id == g_clientID) {
		if (m_avatar->GetState() == AnimationState::Die) m_avatar->SetState(AnimationState::Idle);
		m_avatar->SetPosition(position);
		g_leftX = position.x - 7; g_topY = position.y - 7;
	}
	else {
		if (m_avatar->GetState() == AnimationState::Die) m_avatar->SetState(AnimationState::Idle);
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

void MainScene::SetAnimationInfo(int characterInfo, const shared_ptr<AnimationObject>& object)
{
	switch (characterInfo)
	{
	case Serial::Character::HAKUREI_REIMU:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["REIMU_IDLE"], sf::IntRect{0, 0, 94, 102},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["REIMU_WALK"], sf::IntRect{0, 0, 110, 105},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["REIMU_ATTACK"], sf::IntRect{0, 0, 122, 115},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Skill, AnimationSet{
			g_textures["REIMU_SKILL"], sf::IntRect{0, 0, 102, 100},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["REIMU_DIE"], sf::IntRect{0, 0, 104, 102},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		break;
	case Serial::Character::KONPAKU_YOUMU:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["YOUMU_IDLE"], sf::IntRect{0, 0, 108, 82},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["YOUMU_WALK"], sf::IntRect{0, 0, 112, 73},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["YOUMU_ATTACK"], sf::IntRect{0, 0, 112, 124},
			sf::Vector2i{11, 1}, 0.05f, 0.f
			});
		object->SetAnimationSet(AnimationState::Skill, AnimationSet{
			g_textures["YOUMU_SKILL"], sf::IntRect{0, 0, 122, 84},
			sf::Vector2i{13, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["YOUMU_DIE"], sf::IntRect{0, 0, 110, 110},
			sf::Vector2i{8, 1}, 0.1f, 0.f
			});
		break;
	case Serial::Character::PATCHOULI_KNOWLEDGE:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["PATCHOULI_IDLE"], sf::IntRect{0, 0, 56, 104},
			sf::Vector2i{18, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["PATCHOULI_WALK"], sf::IntRect{0, 0, 46, 99},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Attack, AnimationSet{
			g_textures["PATCHOULI_ATTACK"], sf::IntRect{0, 0, 102, 106},
			sf::Vector2i{7, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Skill, AnimationSet{
			g_textures["PATCHOULI_SKILL"], sf::IntRect{0, 0, 74, 106},
			sf::Vector2i{11, 1}, 0.1f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["PATCHOULI_DIE"], sf::IntRect{0, 0, 106, 82},
			sf::Vector2i{10, 1}, 0.1f, 0.f
			});
		break;
	case Serial::Monster::SHROOM:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["SHROOM_IDLE"], sf::IntRect{0, 0, 36, 36},
			sf::Vector2i{3, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["SHROOM_WALK"], sf::IntRect{0, 0, 36, 36},
			sf::Vector2i{4, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["SHROOM_DIE"], sf::IntRect{0, 0, 42, 37},
			sf::Vector2i{4, 1}, 0.2f, 0.f
			});
		break;
	case Serial::Monster::MUSHROOM:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["MUSHROOM_IDLE"], sf::IntRect{0, 0, 63, 58},
			sf::Vector2i{2, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["MUSHROOM_WALK"], sf::IntRect{0, 0, 66, 66},
			sf::Vector2i{3, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["MUSHROOM_DIE"], sf::IntRect{0, 0, 61, 59},
			sf::Vector2i{3, 1}, 0.2f, 0.f
			});
		break;
	case Serial::Monster::RIBBONPIG:
		object->SetAnimationSet(AnimationState::Idle, AnimationSet{
			g_textures["RIBBONPIG_IDLE"], sf::IntRect{0, 0, 70, 50},
			sf::Vector2i{3, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Walk, AnimationSet{
			g_textures["RIBBONPIG_WALK"], sf::IntRect{0, 0, 67, 50},
			sf::Vector2i{3, 1}, 0.3f, 0.f
			});
		object->SetAnimationSet(AnimationState::Die, AnimationSet{
			g_textures["RIBBONPIG_DIE"], sf::IntRect{0, 0, 73, 56},
			sf::Vector2i{3, 1}, 0.2f, 0.f
			});
		break;
	case Serial::Effect::REIMU_SKILL:
		object->SetAnimationSet(AnimationState::Effect, AnimationSet{
			g_textures["REIMU_EFFECT"], sf::IntRect{0, 0, 132, 138},
			sf::Vector2i{8, 1}, 0.2f, 0.f
			});
		break;
	case Serial::Effect::PATCHOULI_SKILL:
		object->SetAnimationSet(AnimationState::Effect, AnimationSet{
			g_textures["PATCHOULI_EFFECT"], sf::IntRect{0, 0, 186, 178},
			sf::Vector2i{39, 1}, 0.05f, 0.f
			});
		break;
	}
}

void MainScene::LoginInfoProcess(char* buf)
{
	SC_LOGIN_INFO_PACKET* pk = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buf);
	g_clientID = pk->id;
	string hp = "HP : " + to_string(pk->hp) + " / " + to_string(pk->max_hp);
	m_hpUI->SetText(hp.c_str());
	string level = "Level : " + to_string(pk->level);
	m_levelUI->SetText(level.c_str());
	string exp = "EXP : " + to_string(pk->exp) + " / " + to_string((int)pow(2, pk->level - 1) * 100);
	m_expUI->SetText(exp.c_str());
#ifdef NETWORK_DEBUG
	cout << "SC_LOGIN_INFO 수신" << endl;
#endif
}

void MainScene::AddObjectProcess(char* buf)
{
	SC_ADD_OBJECT_PACKET* pk = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(buf);
	sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y };
	AddPlayer(pk->id, pk->serial, pos, pk->name);
#ifdef NETWORK_DEBUG
	cout << "SC_ADD_OBJECT 수신, serial : " << pk->serial << endl;
#endif
}

void MainScene::RemoveObjectProcess(char* buf)
{
	SC_REMOVE_OBJECT_PACKET* pk = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(buf);
	ExitPlayer(pk->id);
#ifdef NETWORK_DEBUG
	cout << "SC_REMOVE_OBJECT 수신" << endl;
#endif
}

void MainScene::MoveObjectProcess(char* buf)
{
	SC_MOVE_OBJECT_PACKET* pk = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(buf);
	sf::Vector2f pos = { (float)pk->coord.x, (float)pk->coord.y };
	Move(pk->id, pos);
#ifdef NETWORK_DEBUG
	cout << "SC_MOVE_OBJECT 수신" << endl;
#endif
}

void MainScene::ChatProcess(char* buf)
{
	SC_CHAT_PACKET* pk = reinterpret_cast<SC_CHAT_PACKET*>(buf);
	string message = to_string(pk->id) + " : " + pk->message;
	SetMessage(message.c_str());
#ifdef NETWORK_DEBUG
	cout << "SC_CHAT 수신" << endl;
#endif
}

void MainScene::StatChangeProcess(char* buf)
{
	SC_STAT_CHANGE_PACKET* pk = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(buf);
	string hp = "HP : " + to_string(pk->hp) + " / " + to_string(pk->max_hp);
	m_hpUI->SetText(hp.c_str());
	string level = "Level : " + to_string(pk->level);
	m_levelUI->SetText(level.c_str());
	string exp = "EXP : " + to_string(pk->exp) + " / " + to_string((int)pow(2, pk->level - 1) * 100);
	m_expUI->SetText(exp.c_str());
#ifdef NETWORK_DEBUG
	cout << "SC_STAT_CHANGE 수신" << endl;
#endif
}

void MainScene::ChangeHpProcess(char* buf)
{
	SC_CHANGE_HP_PACKET* pk = reinterpret_cast<SC_CHANGE_HP_PACKET*>(buf);
	string message = "ID : " + to_string(pk->id) + " HP is " + to_string(pk->hp);
	SetMessage(message.c_str());
#ifdef NETWORK_DEBUG
	cout << "SC_CHANGE_HP 수신" << endl;
#endif
}

void MainScene::DeadObjectProcess(char* buf)
{
	SC_DEAD_OBJECT_PACKET* pk = reinterpret_cast<SC_DEAD_OBJECT_PACKET*>(buf);
	int id = pk->id;
	if (id == g_clientID) m_avatar->SetState(AnimationState::Die);
	else {
		m_players[id]->SetState(AnimationState::Die);
		if (id >= MAX_USER) {
			m_players[id]->SetDeadEvent([&]() {
				ExitPlayer(id);
				});
		}

	}
	string message = "ID : " + to_string(pk->id) + " is Dead ";
	SetMessage(message.c_str());
#ifdef NETWORK_DEBUG
	cout << "SC_DEAD_OBJECT 수신" << endl;
#endif
}

void MainScene::AddEffectProcess(char* buf)
{
	SC_ADD_EFFECT_PACKET* pk = reinterpret_cast<SC_ADD_EFFECT_PACKET*>(buf);

	auto effect = make_shared<EffectObject>(sf::Vector2f{ (float)pk->coord.x, (float)pk->coord.y }, sf::Vector2f{ 1.f, 1.f });
	SetAnimationInfo(pk->serial, effect);
	m_effects.push_back(effect);

#ifdef NETWORK_DEBUG
	cout << "SC_ADD_EFFECT 수신" << endl;
#endif
}

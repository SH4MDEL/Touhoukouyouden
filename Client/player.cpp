#include "player.h"
#include "main.h"

Player::Player(sf::Vector2f position, sf::Vector2f size) : AnimationObject(position, size),
m_chatStatus{ false }, m_chatTime{ 0.f }, m_pressedMoveKey{0.f}, m_direction {2}
{
}

Player::~Player()
{
}

void Player::Update(float timeElapsed)
{
	AnimationObject::Update(timeElapsed);
	if (m_chatStatus) {
		m_chatTime += timeElapsed;
		if (m_chatTime >= m_chatLifeTime) {
			m_chatTime = 0.f;
			m_chatStatus = false;
		}
	}
	m_skillTime -= timeElapsed;
}

void Player::Render(const shared_ptr<sf::RenderWindow>& window)
{
	AnimationObject::Render(window);

	float rx = (m_position.x - g_leftX) * TILE_WIDTH;
	float ry = (m_position.y - g_topY) * TILE_WIDTH;

	auto size = m_name.getGlobalBounds();
	//m_name.setPosition(rx + 32 - size.width / 2, ry + 40);
	m_name.setPosition(rx, ry);
	window->draw(m_name);

	if (m_chatStatus) {
		auto size = m_chat.getGlobalBounds();
		m_chat.setPosition(rx + 32 - size.width / 2, ry - 100);
		window->draw(m_chat);
	}
}

void Player::OnProcessingKeyboardMessage(float timeElapsed)
{
	if (m_state == AnimationState::Attack || 
		m_state == AnimationState::Skill ||
		m_state == AnimationState::Die) return;

	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		SetState(AnimationState::Attack);
		CS_ATTACK_PACKET packet;
		packet.size = sizeof(CS_ATTACK_PACKET);
		packet.type = CS_ATTACK;
		packet.direction = m_direction;
		Send(&packet);
#ifdef NETWORK_DEBUG
		cout << "CS_ATTACK 价脚" << endl;
#endif
		return;
	}
	else if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		if (m_skillTime <= 0) {
			m_skillTime = m_skillCoolTime;
			SetState(AnimationState::Skill);
			CS_SKILL_PACKET packet;
			packet.size = sizeof(CS_SKILL_PACKET);
			packet.type = CS_SKILL;
			packet.direction = m_direction;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_SKILL 价脚" << endl;
#endif
		}
		return;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000 ||
		GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
		m_pressedMoveKey += timeElapsed;
		SetState(AnimationState::Walk);
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		SetSpriteFlip();
		m_direction = 3;
		if (m_pressedMoveKey >= m_moveTime) {
			m_pressedMoveKey -= m_moveTime;
			CS_MOVE_PACKET packet;
			packet.size = sizeof(CS_MOVE_PACKET);
			packet.type = CS_MOVE;
			packet.direction = m_direction;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_MOVE 价脚" << endl;
#endif
		}
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		SetSpriteUnflip();
		m_direction = 2;
		if (m_pressedMoveKey >= m_moveTime) {
			m_pressedMoveKey -= m_moveTime;
			CS_MOVE_PACKET packet;
			packet.size = sizeof(CS_MOVE_PACKET);
			packet.type = CS_MOVE;
			packet.direction = m_direction;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_MOVE 价脚" << endl;
#endif
		}
	}
	else if (GetAsyncKeyState(VK_UP) & 0x8000) {
		m_direction = 1;
		if (m_pressedMoveKey >= m_moveTime) {
			m_pressedMoveKey -= m_moveTime;
			CS_MOVE_PACKET packet;
			packet.size = sizeof(CS_MOVE_PACKET);
			packet.type = CS_MOVE;
			packet.direction = m_direction;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_MOVE 价脚" << endl;
#endif
		}
	}
	else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		m_direction = 0;
		if (m_pressedMoveKey >= m_moveTime) {
			m_pressedMoveKey -= m_moveTime;
			CS_MOVE_PACKET packet;
			packet.size = sizeof(CS_MOVE_PACKET);
			packet.type = CS_MOVE;
			packet.direction = m_direction;
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_MOVE 价脚" << endl;
#endif
		}
	}
	else {
		m_pressedMoveKey = 0.f;
		SetState(AnimationState::Idle);
	}
}

void Player::SetName(const char* name)
{
	m_name.setFont(g_font);
	m_name.setString(name);
	m_name.setFillColor(sf::Color(0, 0, 0));
	m_name.setStyle(sf::Text::Bold);
}

void Player::SetChat(const char* chat)
{
	m_chatStatus = true;
	m_chatTime = 0.f;
	m_chat.setFont(g_font);
	m_chat.setString(chat);
	m_chat.setFillColor(sf::Color(0, 255, 255));
	m_chat.setStyle(sf::Text::Bold);
}

void Player::SetSerial(const int serial)
{
	if (serial == Serial::Character::HAKUREI_REIMU) {
		m_moveTime = StatusSetting::REIMU::MOVE_SPEED;
		m_skillCoolTime = StatusSetting::REIMU::SKILL_COOLTIME;
	}
	if (serial == Serial::Character::KONPAKU_YOUMU) {
		m_moveTime = StatusSetting::YOUMU::MOVE_SPEED;
		m_skillCoolTime = StatusSetting::YOUMU::SKILL_COOLTIME;
	}
	if (serial == Serial::Character::PATCHOULI_KNOWLEDGE) {
		m_moveTime = StatusSetting::PATCHOULI::MOVE_SPEED;
		m_skillCoolTime = StatusSetting::PATCHOULI::SKILL_COOLTIME;
	}
}

#include "player.h"

Player::Player(sf::Vector2f position, sf::Vector2f size) : AnimationObject(position, size),
	m_chatStatus{ false }, m_chatTime {0.f}
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

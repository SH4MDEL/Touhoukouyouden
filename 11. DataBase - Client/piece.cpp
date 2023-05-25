#include "piece.h"

Piece::Piece(Short2 position, Short2 length) : Object(position, length), 
	m_chatStatue{ false }, m_chatTime {0.f}
{
}

Piece::~Piece()
{
}

void Piece::Update(float timeElapsed)
{
	if (m_chatStatue) {
		m_chatTime += timeElapsed;
		if (m_chatTime >= m_chatLifeTime) {
			m_chatTime = 0.f;
			m_chatStatue = false;
		}
	}
}

void Piece::Render(const shared_ptr<sf::RenderWindow>& window)
{
	float rx = (m_position.x - g_leftX) * TILE_WIDTH;
	float ry = (m_position.y - g_topY) * TILE_WIDTH;
	m_sprite.setPosition(rx, ry);
	window->draw(m_sprite);
	if (m_chatStatue) {
		auto size = m_chat.getGlobalBounds();
		m_chat.setPosition(rx + 32 - size.width / 2, ry - 20);
		window->draw(m_chat);
	}
	else {
		auto size = m_name.getGlobalBounds();
		m_name.setPosition(rx + 32 - size.width / 2, ry - 20);
		window->draw(m_name);
	}
}

void Piece::SetPosition(Short2 position)
{
	m_position = position;
}

void Piece::SetName(const char* name)
{
	m_name.setFont(g_font);
	m_name.setString(name);
	m_name.setFillColor(sf::Color(255, 255, 0));
	m_name.setStyle(sf::Text::Bold);
}

void Piece::SetChat(const char* chat)
{
	m_chatStatue = true;
	m_chatTime = 0.f;
	m_chat.setFont(g_font);
	m_chat.setString(chat);
	m_chat.setFillColor(sf::Color(0, 255, 255));
	m_chat.setStyle(sf::Text::Bold);
}

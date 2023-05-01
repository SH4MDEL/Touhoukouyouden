#include "piece.h"

Piece::Piece(POINT position, POINT length) : Object(position, length)
{
}

Piece::~Piece()
{
}

void Piece::Render(const shared_ptr<sf::RenderWindow>& window)
{
	float rx = (m_position.x - g_leftX) * TILE_WIDTH;
	float ry = (m_position.y - g_topY) * TILE_WIDTH;
	m_sprite.setPosition(rx, ry);
	window->draw(m_sprite);
	//window->draw(m_name);
}

void Piece::SetPosition(POINT position)
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
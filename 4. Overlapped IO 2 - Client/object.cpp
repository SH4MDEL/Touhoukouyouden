#include "Object.h"

Object::Object(POINT position, POINT length) : m_position(position), m_length(length) {}

void Object::Update(float timeElapsed) {}

void Object::Render(const shared_ptr<sf::RenderWindow>& window)
{
	window->draw(m_sprite);
}

void Object::SetPosition(POINT position)
{
	m_position = position;
	m_sprite.setPosition(position.x, position.y);
}

void Object::SetTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy)
{
	m_sprite.setTexture(*(texture.get()));
	m_sprite.setTextureRect(sf::IntRect(x, y, dx, dy));
}

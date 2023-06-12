#include "Object.h"

Object::Object(sf::Vector2f position, sf::Vector2f size) : 
	m_spritePosition(position), m_spriteSize(size) {}

Object::~Object()
{
}

void Object::Update(float timeElapsed) {}

void Object::Render(const shared_ptr<sf::RenderWindow>& window)
{
	window->draw(m_sprite);
}

void Object::SetPosition(sf::Vector2f position)
{
	m_spritePosition = position;
	m_sprite.setPosition(position);
}

// size -> scale을 통해 크기를 결정해야 함
// length -> 절대 크기를 통해 크기를 결정해야 함
void Object::SetSpriteSize(sf::Vector2f size)
{
	m_spriteSize = size;
	m_sprite.setScale(size);
}

void Object::SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy)
{
	// 텍스처 내에서 어떤 부분이 그려질지를 결정
	// 스프라이트 이미지를 사용할 때 수정할 필요 있음.
	m_sprite.setTexture(*(texture.get()));
	m_sprite.setTextureRect(sf::IntRect(x, y, dx, dy));

	// 이미 할당된 위치와 크기 적용
	m_sprite.setPosition(m_spritePosition);
	m_sprite.setScale(m_spriteSize);
}

sf::Vector2f Object::GetPosition()
{
	return m_spritePosition;
}

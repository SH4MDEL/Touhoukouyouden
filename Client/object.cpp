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

// size -> scale�� ���� ũ�⸦ �����ؾ� ��
// length -> ���� ũ�⸦ ���� ũ�⸦ �����ؾ� ��
void Object::SetSpriteSize(sf::Vector2f size)
{
	m_spriteSize = size;
	m_sprite.setScale(size);
}

void Object::SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy)
{
	// �ؽ�ó ������ � �κ��� �׷������� ����
	// ��������Ʈ �̹����� ����� �� ������ �ʿ� ����.
	m_sprite.setTexture(*(texture.get()));
	m_sprite.setTextureRect(sf::IntRect(x, y, dx, dy));

	// �̹� �Ҵ�� ��ġ�� ũ�� ����
	m_sprite.setPosition(m_spritePosition);
	m_sprite.setScale(m_spriteSize);
}

sf::Vector2f Object::GetPosition()
{
	return m_spritePosition;
}

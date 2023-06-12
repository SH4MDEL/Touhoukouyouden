#pragma once
#include "stdafx.h"

class Object
{
public:
	Object() = default;
	Object(sf::Vector2f position, sf::Vector2f size);
	virtual ~Object();

	virtual void Update(float timeElapsed);
	virtual void Render(const shared_ptr<sf::RenderWindow>& window);

	virtual void SetPosition(sf::Vector2f position);
	virtual void SetSpriteSize(sf::Vector2f size);
	virtual void SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy);

	sf::Vector2f GetPosition();

protected:
	sf::Sprite				m_sprite;
	sf::Vector2f			m_spritePosition;
	sf::Vector2f			m_spriteSize;
};


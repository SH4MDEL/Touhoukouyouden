#pragma once
#include "stdafx.h"

class Object
{
public:
	Object() = default;
	Object(Short2 position, Short2 length);
	virtual ~Object() = default;

	virtual void Update(float timeElapsed);
	virtual void Render(const shared_ptr<sf::RenderWindow>& window);

	virtual void SetPosition(Short2 position);
	virtual void SetLength(Short2 length) { m_length = length; }
	virtual void SetTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy);

	Short2 GetPosition();

protected:
	Short2					m_position;
	Short2					m_length;

	sf::Sprite				m_sprite;
};


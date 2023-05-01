#pragma once
#include "stdafx.h"

class Object
{
public:
	Object() = default;
	Object(POINT position, POINT length);
	virtual ~Object() = default;

	virtual void Update(float timeElapsed);
	virtual void Render(const shared_ptr<sf::RenderWindow>& window);

	virtual void SetPosition(POINT position);
	virtual void SetLength(POINT length) { m_length = length; }
	virtual void SetTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy);

protected:
	POINT					m_position;
	POINT					m_length;

	sf::Sprite				m_sprite;
};


#pragma once
#include "object.h"

class Piece : public Object
{
public:
	Piece() = default;
	Piece(Short2 position, Short2 length);
	~Piece() override;

	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetPosition(Short2 position) override;
	virtual void SetName(const char* name);

private:
	sf::Text					m_name;
};


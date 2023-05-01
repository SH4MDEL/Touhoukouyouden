#pragma once
#include "object.h"

class Piece : public Object
{
public:
	Piece() = default;
	Piece(POINT position, POINT length);
	~Piece() override;

	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetPosition(POINT position) override;
	virtual void SetName(const char* name);

private:
	sf::Text					m_name;
};


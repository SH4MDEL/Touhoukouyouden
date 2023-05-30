#pragma once
#include "object.h"

class Piece : public Object
{
public:
	Piece() = default;
	Piece(sf::Vector2f position, sf::Vector2f size);
	~Piece() override;

	virtual void Update(float timeElapsed) override;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetName(const char* name);
	void SetChat(const char* name);
private:
	sf::Text					m_name;
	sf::Text					m_chat;
	BOOL						m_chatStatue;
	const FLOAT					m_chatLifeTime = 2.f;
	FLOAT						m_chatTime;
};


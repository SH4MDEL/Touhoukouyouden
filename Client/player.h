#pragma once
#include "object.h"

class Player : public AnimationObject
{
public:
	Player() = default;
	Player(sf::Vector2f position, sf::Vector2f size);
	~Player() override;

	virtual void Update(float timeElapsed) override;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetName(const char* name);
	void SetChat(const char* name);
private:
	sf::Text					m_name;
	sf::Text					m_chat;
	BOOL						m_chatStatus;
	const FLOAT					m_chatLifeTime = 2.f;
	FLOAT						m_chatTime;
};


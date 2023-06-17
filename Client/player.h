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

	void OnProcessingKeyboardMessage(float timeElapsed);

	virtual void SetName(const char* name);
	void SetChat(const char* name);
private:
	sf::Text		m_name;
	sf::Text		m_chat;
	BOOL			m_chatStatus;
	const float		m_chatLifeTime = 2.f;
	FLOAT			m_chatTime;

	const float		m_moveTime = 0.15f;
	float			m_pressedMoveKey;
	unsigned char	m_direction;
};


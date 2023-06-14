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
	virtual void SetSize(sf::Vector2f size);
	virtual void SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy);

	sf::Vector2f GetPosition();

protected:
	sf::Sprite				m_sprite;
	sf::Vector2f			m_position;
	sf::Vector2f			m_size;
};

struct AnimationSet
{
public:
	AnimationSet() = default;
	AnimationSet(const shared_ptr<sf::Texture>& texture, sf::IntRect spriteRect, sf::Vector2i spriteNum,
		float animationLifetime, float animationTime) :
		m_spriteRect{ spriteRect }, m_spriteNum{ spriteNum },
		m_animationLifetime{ animationLifetime }, m_animationTime{ animationTime }
	{
		m_sprite.setTexture(*(texture.get()));
		m_sprite.setTextureRect(spriteRect);
	}


public:
	sf::Sprite				m_sprite;
	sf::IntRect				m_spriteRect;
	sf::Vector2i			m_spriteNum;
	float					m_animationLifetime;
	float					m_animationTime;
};

class AnimationObject : public Object
{
public:
	AnimationObject() = default;
	AnimationObject(sf::Vector2f position, sf::Vector2f size);
	virtual ~AnimationObject() override;

	void Update(float timeElapsed) override;
	void Render(const shared_ptr<sf::RenderWindow>& window) override;

	void SetAnimationSet(AnimationState state, const AnimationSet& animationSet);
	void SetState(AnimationState state);

protected:
	array<AnimationSet, AnimationState::Count>	m_animationSet;
	AnimationState								m_state;
};
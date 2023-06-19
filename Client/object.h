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
	virtual void SetSpriteColor(sf::Color color);

	virtual void SetSpriteFlip();
	virtual void SetStriteUnflip();

	sf::Vector2f GetPosition();

protected:
	sf::Sprite				m_sprite;
	sf::Vector2f			m_position;
	sf::Vector2f			m_size;
	
	bool					m_flipped;
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

	virtual void Update(float timeElapsed) override;
	void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetSpriteFlip();
	virtual void SetSpriteUnflip();

	void SetAnimationSet(AnimationState state, const AnimationSet& animationSet);
	void SetState(AnimationState state);
	AnimationState GetState();

	void SetDeadEvent(const function<void()>& deadEvent);

protected:
	array<AnimationSet, AnimationState::Count>	m_animationSet;
	AnimationState								m_state;

	AnimationState								m_prevState;

	function<void()>		m_deadEvent;
};

class EffectObject : public AnimationObject
{
public:
	EffectObject(sf::Vector2f position, sf::Vector2f size);
	virtual ~EffectObject() override;

	virtual void Update(float timeElapsed) override;

	bool IsFinish() { return m_finish; }

private:
	bool m_finish;
};
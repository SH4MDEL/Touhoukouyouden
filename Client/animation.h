#pragma once
#include "stdafx.h"

// 스프라이트 이미지를 출력하기 위해 필요한 정보를 저장한다.
struct AnimationInfo
{
public:
	AnimationInfo() = default;
	AnimationInfo(const shared_ptr<sf::Texture>& texture, sf::Vector2i spriteLength, sf::Vector2i spriteNum,
		float animationLifetime, float animationTime) :
		m_spriteLength{ spriteLength }, m_spriteNum{ spriteNum },
		m_animationLifetime{ animationLifetime }, m_animationTime{ animationTime }
	{
		m_sprite.setTexture(*(texture.get()));
		m_sprite.setTextureRect(spriteRect);
	}

	void Render(const shared_ptr<sf::RenderWindow>& window, sf::IntRect& spriteRect, sf::Vector2i position);
	void NextImage(sf::IntRect& spriteRect);

public:
	sf::Sprite				m_sprite;
	sf::Vector2i			m_spriteLength;
	sf::Vector2i			m_spriteNum;
	float					m_animationLifetime;
	float					m_animationTime;
};

// 하나의 오브젝트의 상태에 대한 스프라이트 이미지들의 모임이다.
class AnimationSet
{
public:
	AnimationSet();

	enum State {
		IDLE,
		MOVE,
		ATTACK,
		DIE,
		COUNT
	};

	void Render(const shared_ptr<sf::RenderWindow>& window, sf::IntRect& spriteRect, sf::Vector2i position);

	void SetAnimationInfo(const shared_ptr<AnimationInfo>& animationInfo, State state);
	void SetState(State state);

private:
	array<shared_ptr<AnimationInfo>, State::COUNT> m_animationInfo;
	State m_state;
};
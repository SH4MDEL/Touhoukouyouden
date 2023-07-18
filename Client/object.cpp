#include "Object.h"

Object::Object(sf::Vector2f position, sf::Vector2f size) : 
	m_position(position), m_size(size), m_flipped{ false } {}

Object::~Object()
{
}

void Object::Update(float timeElapsed) {}

void Object::Render(const shared_ptr<sf::RenderWindow>& window)
{
	window->draw(m_sprite);
}

void Object::SetPosition(sf::Vector2f position)
{
	m_position = position;
	m_sprite.setPosition(position);
}

// size -> scale을 통해 크기를 결정해야 함
// length -> 절대 크기를 통해 크기를 결정해야 함
void Object::SetSize(sf::Vector2f size)
{
	m_size = size;
	m_sprite.setScale(size);
}

void Object::SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy)
{
	// 텍스처 내에서 어떤 부분이 그려질지를 결정
	// 스프라이트 이미지를 사용할 때 수정할 필요 있음.
	m_sprite.setTexture(*(texture.get()));
	m_sprite.setTextureRect(sf::IntRect(x, y, dx, dy));

	// 이미 할당된 위치와 크기 적용
	m_sprite.setPosition(m_position);
	m_sprite.setScale(m_size);
}

void Object::SetSpriteColor(sf::Color color)
{
	m_sprite.setColor(color);
}

void Object::SetSpriteFlip()
{
	if (!m_flipped) {
		auto flipSize = m_size;
		flipSize.x *= -1;
		m_sprite.setScale(flipSize);
		m_flipped = true;
	}
}

void Object::SetStriteUnflip()
{
	if (m_flipped) {
		m_sprite.setScale(m_size);
		m_flipped = false;
	}
}

sf::Vector2f Object::GetPosition()
{
	return m_position;
}



AnimationObject::AnimationObject(sf::Vector2f position, sf::Vector2f size) : 
	Object(position, size), m_state{AnimationState::Idle}, m_prevState{ AnimationState::Idle }
{
}

AnimationObject::~AnimationObject()
{
}

void AnimationObject::Update(float timeElapsed)
{
	m_animationSet[m_state].m_animationTime += timeElapsed;
	if (m_animationSet[m_state].m_animationTime >= m_animationSet[m_state].m_animationLifetime) {
		m_animationSet[m_state].m_animationTime -= m_animationSet[m_state].m_animationLifetime;

		// 세로 끝이면서 가로 끝이면 초기로 돌린다.
		if (m_animationSet[m_state].m_spriteRect.top / m_animationSet[m_state].m_spriteRect.height + 1 == m_animationSet[m_state].m_spriteNum.y &&
			m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			// 공격 모션은 한 번만 재생하고 되돌린다.
			if (m_state == AnimationState::Attack || m_state == AnimationState::Skill) {
				SetState(m_prevState);
			}
			// 사망 모션 종료시 DeadEvent 실행
			else if (m_state == AnimationState::Die) {
				if (m_deadEvent) m_deadEvent();
				return;
			}

			m_animationSet[m_state].m_spriteRect.top = 0;
			m_animationSet[m_state].m_spriteRect.left = 0;
		}
		// 가로 끝이면 세로 1 늘리고 가로 초기화한다.
		else if (m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			m_animationSet[m_state].m_spriteRect.top += m_animationSet[m_state].m_spriteRect.height;
			m_animationSet[m_state].m_spriteRect.left = 0;
		}
		// 아니면 가로 1 늘린다.
		else {
			m_animationSet[m_state].m_spriteRect.left += m_animationSet[m_state].m_spriteRect.width;
		}

		// 바뀐 정보를 반영한다.
		m_animationSet[m_state].m_sprite.setTextureRect(m_animationSet[m_state].m_spriteRect);
	}
}

void AnimationObject::Render(const shared_ptr<sf::RenderWindow>& window)
{
	float rx = (m_position.x - g_leftX) * TILE_WIDTH;
	float ry = (m_position.y - g_topY) * TILE_WIDTH;

	auto size = m_animationSet[m_state].m_sprite.getGlobalBounds();
	if (m_flipped) rx += (float)m_animationSet[m_state].m_spriteRect.width;
	m_animationSet[m_state].m_sprite.setPosition(rx + 32 - size.width / 2, ry + 10 - size.height / 2);

	window->draw(m_animationSet[m_state].m_sprite);
}

void AnimationObject::SetSpriteFlip()
{
	if (!m_flipped) {
		auto flipSize = m_size;
		flipSize.x *= -1;
		for (auto& animationSet : m_animationSet) {
			animationSet.m_sprite.setScale(flipSize);
		}
		m_flipped = true;
	}
}

void AnimationObject::SetSpriteUnflip()
{
	if (m_flipped) {
		for (auto& animationSet : m_animationSet) {
			animationSet.m_sprite.setScale(m_size);
		}
		m_flipped = false;
	}
}

void AnimationObject::SetAnimationSet(AnimationState state, const AnimationSet& animationSet)
{
	m_animationSet[state] = animationSet;
}

void AnimationObject::SetState(AnimationState state)
{
	if (m_state != state) {
		m_state = state;
		m_animationSet[m_state].m_animationTime = 0.f;
		m_animationSet[m_state].m_spriteRect.top = 0;
		m_animationSet[m_state].m_spriteRect.left = 0;
	}
}

AnimationState AnimationObject::GetState()
{
	return m_state;
}

void AnimationObject::SetDeadEvent(const function<void()>& deadEvent)
{
	m_deadEvent = deadEvent;
}

EffectObject::EffectObject(sf::Vector2f position, sf::Vector2f size) :
	AnimationObject(position, size), m_finish{false}
{
	m_state = AnimationState::Effect;
}

EffectObject::~EffectObject()
{
}

void EffectObject::Update(float timeElapsed)
{
	m_animationSet[m_state].m_animationTime += timeElapsed;
	if (m_animationSet[m_state].m_animationTime >= m_animationSet[m_state].m_animationLifetime) {
		m_animationSet[m_state].m_animationTime -= m_animationSet[m_state].m_animationLifetime;

		// 세로 끝이면서 가로 끝이면 삭제
		if (m_animationSet[m_state].m_spriteRect.top / m_animationSet[m_state].m_spriteRect.height + 1 == m_animationSet[m_state].m_spriteNum.y &&
			m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			m_finish = true;
			return;
		}
		// 가로 끝이면 세로 1 늘리고 가로 초기화한다.
		else if (m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			m_animationSet[m_state].m_spriteRect.top += m_animationSet[m_state].m_spriteRect.height;
			m_animationSet[m_state].m_spriteRect.left = 0;
		}
		// 아니면 가로 1 늘린다.
		else {
			m_animationSet[m_state].m_spriteRect.left += m_animationSet[m_state].m_spriteRect.width;
		}

		// 바뀐 정보를 반영한다.
		m_animationSet[m_state].m_sprite.setTextureRect(m_animationSet[m_state].m_spriteRect);
	}
}
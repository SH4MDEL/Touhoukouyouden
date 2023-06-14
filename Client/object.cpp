#include "Object.h"

Object::Object(sf::Vector2f position, sf::Vector2f size) : 
	m_position(position), m_size(size) {}

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

// size -> scale�� ���� ũ�⸦ �����ؾ� ��
// length -> ���� ũ�⸦ ���� ũ�⸦ �����ؾ� ��
void Object::SetSize(sf::Vector2f size)
{
	m_size = size;
	m_sprite.setScale(size);
}

void Object::SetSpriteTexture(const shared_ptr<sf::Texture>& texture, INT x, INT y, INT dx, INT dy)
{
	// �ؽ�ó ������ � �κ��� �׷������� ����
	// ��������Ʈ �̹����� ����� �� ������ �ʿ� ����.
	m_sprite.setTexture(*(texture.get()));
	m_sprite.setTextureRect(sf::IntRect(x, y, dx, dy));

	// �̹� �Ҵ�� ��ġ�� ũ�� ����
	m_sprite.setPosition(m_position);
	m_sprite.setScale(m_size);
}

sf::Vector2f Object::GetPosition()
{
	return m_position;
}



AnimationObject::AnimationObject(sf::Vector2f position, sf::Vector2f size) : 
	Object(position, size), m_state{AnimationState::Idle}
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

		// ���� ���̸鼭 ���� ���̸� �ʱ�� ������.
		if (m_animationSet[m_state].m_spriteRect.top / m_animationSet[m_state].m_spriteRect.height + 1 == m_animationSet[m_state].m_spriteNum.y &&
			m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			// ���� ����� �� ���� ����ϰ� Idle�� ������.
			if (m_state == AnimationState::Attack) {
				SetState(AnimationState::Idle);
			}
			// ��� ����� �ݺ����� �ʰ� ������ �̹����� ��� ����Ѵ�.
			else if (m_state == AnimationState::Die) return;

			m_animationSet[m_state].m_spriteRect.top = 0;
			m_animationSet[m_state].m_spriteRect.left = 0;
		}
		// ���� ���̸� ���� 1 �ø��� ���� �ʱ�ȭ�Ѵ�.
		else if (m_animationSet[m_state].m_spriteRect.left / m_animationSet[m_state].m_spriteRect.width + 1 == m_animationSet[m_state].m_spriteNum.x) {
			m_animationSet[m_state].m_spriteRect.top += m_animationSet[m_state].m_spriteRect.height;
			m_animationSet[m_state].m_spriteRect.left = 0;
		}
		// �ƴϸ� ���� 1 �ø���.
		else {
			m_animationSet[m_state].m_spriteRect.left += m_animationSet[m_state].m_spriteRect.width;
		}

		// �ٲ� ������ �ݿ��Ѵ�.
		m_animationSet[m_state].m_sprite.setTextureRect(m_animationSet[m_state].m_spriteRect);
	}
}

void AnimationObject::Render(const shared_ptr<sf::RenderWindow>& window)
{
	float rx = (m_position.x - g_leftX) * TILE_WIDTH;
	float ry = (m_position.y - g_topY) * TILE_WIDTH;
	m_animationSet[m_state].m_sprite.setPosition(rx, ry);
	window->draw(m_animationSet[m_state].m_sprite);
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
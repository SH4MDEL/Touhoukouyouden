#include "animation.h"

void AnimationInfo::Render(const shared_ptr<sf::RenderWindow>& window, sf::IntRect& spriteRect, sf::Vector2i position)
{
	m_animationInfo[m_state]
}


AnimationSet::AnimationSet() : m_state{State::IDLE}
{
}

void AnimationSet::Render(const shared_ptr<sf::RenderWindow>& window, sf::IntRect& spriteRect, sf::Vector2i position)
{
	m_animationInfo[m_state]->Render(window, spriteRect, position);
}

void AnimationSet::SetAnimationInfo(const shared_ptr<AnimationInfo>& animationInfo, State state)
{
	m_animationInfo[state] = animationInfo;
}

void AnimationSet::SetState(State state)
{
	if (m_state != state) {
		m_state;
		m_animationInfo[m_state]->m_animationTime = 0.f;
		m_animationInfo[m_state]->m_spriteRect.top = 0;
		m_animationInfo[m_state]->m_spriteRect.left = 0;
	}
}

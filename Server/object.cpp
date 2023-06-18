#include "object.h"

OBJECT::OBJECT() :
	m_state{ State::FREE }, m_id{ -1 }, m_serial{ -1 }, m_lastMoveTime{ 0 },
	m_position{ 0, 0 }, m_prevRemain{ 0 }
{}

bool OBJECT::CanSee(Short2 position)
{
	if (std::abs(m_position.x - position.x) > VIEW_RANGE) return false;
	return std::abs(m_position.y - position.y) <= VIEW_RANGE;
}

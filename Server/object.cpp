#include "object.h"

OBJECT::OBJECT() :
	m_state{ State::FREE }, m_id{ -1 }, m_lastMoveTime{ 0 },
	m_position{ 0, 0 }, m_prevRemain{ 0 }
{}
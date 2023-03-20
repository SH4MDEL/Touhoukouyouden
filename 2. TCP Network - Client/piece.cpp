#include "piece.h"

Piece::Piece(POINT boradPosition, POINT length) : m_boardPosition{ boradPosition }, Object({ 0, 0 }, length)
{
}

Piece::~Piece()
{
}

void Piece::Move(POINT to)
{
	m_boardPosition = to;
}

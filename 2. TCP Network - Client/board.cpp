#include "board.h"

Board::Board(POINT position, POINT length) : Object(position, length)
{
}

Board::~Board()
{
}

void Board::Render(HDC hdc)
{
	Object::Render(hdc);
	for (const auto& piece : m_pieces) {
		auto objectBoardPosition = piece->GetBoardPosition();
		piece->Render(hdc, { m_position.x - m_length.x / 2 + m_length.x / 8 * objectBoardPosition.x,  
			m_position.y - m_length.y / 2 + m_length.y / 8 * objectBoardPosition.y });
	}
}

void Board::Move(POINT from, POINT to)
{
	auto fromX = from.x;
	auto fromY = from.y;
	for (const auto& piece : m_pieces) {
		auto ObjectBoardPosition = piece->GetBoardPosition();
		if (ObjectBoardPosition.x == fromX && ObjectBoardPosition.y == fromY) {
			piece->Move(to);
		}
	}
}

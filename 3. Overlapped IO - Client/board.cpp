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
	for (const auto& piece : m_players) {
		auto objectBoardPosition = piece.second->GetBoardPosition();
		piece.second->Render(hdc, { m_position.x - m_length.x / 2 + m_length.x / 8 * objectBoardPosition.x,
			m_position.y - m_length.y / 2 + m_length.y / 8 * objectBoardPosition.y });
		//piece.second->Render(hdc, { m_position.y - m_length.y / 2 + m_length.y / 8 * objectBoardPosition.y,
		//	m_position.x - m_length.x / 2 + m_length.x / 8 * objectBoardPosition.x });
	}
}

void Board::Move(POINT from, POINT to)
{
	auto fromX = from.x;
	auto fromY = from.y;
	for (const auto& piece : m_players) {
		auto ObjectBoardPosition = piece.second->GetBoardPosition();
		if (ObjectBoardPosition.x == fromX && ObjectBoardPosition.y == fromY) {
			piece.second->Move(to);
		}
	}
}

void Board::Move(int id, POINT to)
{
	m_players[id]->Move(to);
}

void Board::ExitPlayer(int id)
{
	if (m_players.find(id) != m_players.end()) {
		m_players.erase(id);
	}
}

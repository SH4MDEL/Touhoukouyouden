#pragma once
#include "object.h"
#include "board.h"

class Board;
class Piece : public Object
{
public:
	Piece() = default;
	Piece(POINT position, POINT length);
	~Piece() override;

	void Move(POINT to);

	void SetBoard(shared_ptr<Board> board) { m_board = board; }
	void SetBoardPosition(POINT boardPosition) { m_boardPosition = boardPosition; }

	POINT GetBoardPosition() const { return m_boardPosition; }

private:
	POINT				m_boardPosition;
	shared_ptr<Board>	m_board;

};


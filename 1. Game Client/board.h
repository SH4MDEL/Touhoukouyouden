#pragma once
#include "object.h"
#include "piece.h"

class Piece;

class Board : public Object
{
public:
	Board() = default;
	Board(POINT position, POINT length);
	~Board() override;

	void Render(HDC hdc) override;
	void Move(POINT from, POINT to);

	void SetPiece(shared_ptr<Piece> piece) { m_pieces.push_back(piece); }

private:
	vector<shared_ptr<Piece>>	m_pieces;
};


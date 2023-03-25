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
	void Move(int id, POINT to);

	void SetPlayer(int id, shared_ptr<Piece> piece) { m_players.insert({id, piece}); }
	void ExitPlayer(int id);

private:
	unordered_map<int, shared_ptr<Piece>> m_players;
};


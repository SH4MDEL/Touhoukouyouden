#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "..\Server\protocol.h"

using namespace std;

mt19937 g_randomEngine;
inline INT GetRandomINT(INT min, INT max)
{
	uniform_int_distribution<INT> dis{ min, max };
	return dis(g_randomEngine);
}

bool CreateHenesys(int x, int y, ofstream& out)
{
	using namespace VillageBorder;
	// 헤네시스 필드가 아니면 리턴
	if (x < HENESYS_START.first || x >= HENESYS_END.first) return false;
	if (y < HENESYS_START.second || x >= HENESYS_END.second) return false;

	// 마을과 사냥터의 경계는 이동 불가능한 벽으로 둘러침
	// 
	// 왼쪽 벽 생성
	if (x == HENESYS_VILLAGE_START.first &&
		(y >= HENESYS_VILLAGE_START.second && y < HENESYS_VILLAGE_END.second)) {
		// 왼쪽 벽 입구
		if ((y >= HENESYS_VILLAGE_START.second +
			(HENESYS_VILLAGE_END.second - HENESYS_VILLAGE_START.second) / 2 - 5) &&
			(y < HENESYS_VILLAGE_START.second +
				(HENESYS_VILLAGE_END.second - HENESYS_VILLAGE_START.second) / 2 + 5)) {
			out << TileInfo::HENESYS_NONBLOCK;
			return true;
		}
		else {
			out << TileInfo::HENESYS_BLOCK;
			return true;
		}
	}
	// 오른쪽 벽 생성
	else if (x == HENESYS_VILLAGE_END.first &&
		(y >= HENESYS_VILLAGE_START.second && y < HENESYS_VILLAGE_END.second)) {
		// 오른쪽 벽 입구
		if ((y >= HENESYS_VILLAGE_START.second +
			(HENESYS_VILLAGE_END.second - HENESYS_VILLAGE_START.second) / 2 - 5) &&
			(y < HENESYS_VILLAGE_START.second +
				(HENESYS_VILLAGE_END.second - HENESYS_VILLAGE_START.second) / 2 + 5)) {
			out << TileInfo::HENESYS_NONBLOCK;
			return true;
		}
		else {
			out << TileInfo::HENESYS_BLOCK;
			return true;
		}
	}
	// 위쪽 벽 생성
	else if (y == HENESYS_VILLAGE_START.second &&
		(x >= HENESYS_VILLAGE_START.first && x < HENESYS_VILLAGE_END.first)) {
		// 위쪽 벽 입구
		if ((x >= HENESYS_VILLAGE_START.first +
			(HENESYS_VILLAGE_END.first - HENESYS_VILLAGE_START.first) / 2 - 5) &&
			(x < HENESYS_VILLAGE_START.first +
				(HENESYS_VILLAGE_END.first - HENESYS_VILLAGE_START.first) / 2 + 5)) {
			out << TileInfo::HENESYS_NONBLOCK;
			return true;
		}
		else {
			out << TileInfo::HENESYS_BLOCK;
			return true;
		}
	}
	// 아래쪽 벽 생성
	else if (y == HENESYS_VILLAGE_END.second &&
		(x >= HENESYS_VILLAGE_START.first && x < HENESYS_VILLAGE_END.first)) {
		// 아래쪽 벽 입구
		if ((x >= HENESYS_VILLAGE_START.first +
			(HENESYS_VILLAGE_END.first - HENESYS_VILLAGE_START.first) / 2 - 5) &&
			(x < HENESYS_VILLAGE_START.first +
				(HENESYS_VILLAGE_END.first - HENESYS_VILLAGE_START.first) / 2 + 5)) {
			out << TileInfo::HENESYS_NONBLOCK;
			return true;
		}
		else {
			out << TileInfo::HENESYS_BLOCK;
			return true;
		}
	}
	// 마을의 내부일 경우
	else if (x >= VillageBorder::HENESYS_VILLAGE_START.first && x < VillageBorder::HENESYS_VILLAGE_END.first &&
		y >= VillageBorder::HENESYS_VILLAGE_START.second && y < VillageBorder::HENESYS_VILLAGE_END.second) {
		out << TileInfo::HENESYS_NONBLOCK;
		return true;
	}
	// 마을의 외부일 경우
	else {
		if (GetRandomINT(1, 10) == 1) {
			out << TileInfo::HENESYS_BLOCK;
			return true;
		}
		else {
			out << TileInfo::HENESYS_NONBLOCK;
			return true;
		}
	}
	return false;
}

int main()
{
	ofstream out("map.txt");

	for (int i = 0; i < W_HEIGHT; ++i) {
		for (int j = 0; j < W_WIDTH; ++j) {
			if (CreateHenesys(j, i, out)) {}
			else {
				if (GetRandomINT(1, 10) == 1) {
					out << TileInfo::UNDEFINED_BLOCK;
				}
				else {
					out << TileInfo::UNDEFINED_NONBLOCK;
				}
			}
			out << " ";
		}
	}
	
}
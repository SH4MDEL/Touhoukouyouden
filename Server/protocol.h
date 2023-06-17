#pragma once
#include <windows.h>
#include <utility>

#define NETWORK_DEBUG

constexpr int		PORT_NUM = 4000;
constexpr int		NAME_SIZE = 20;
constexpr int		CHAT_SIZE = 200;
constexpr int		BUF_SIZE = 1024;

constexpr int		ID_SIZE = 20;
constexpr int		PASSWORD_SIZE = 20;

constexpr int		MAX_USER = 20000;
constexpr int		MAX_MONSTER = 5000;

constexpr int		W_WIDTH = 2000;
constexpr int		W_HEIGHT = 2000;

// Packet ID
constexpr char		CS_LOGIN = 0;
constexpr char		CS_MOVE = 1;
constexpr char		CS_CHAT = 2;
constexpr char		CS_ATTACK = 3;
constexpr char		CS_TELEPORT = 4; // RANDOM한 위치로 Teleport, Stress Test할 때 Hot Spot현상을 피하기 위해 구현
constexpr char		CS_LOGOUT = 5;

constexpr char		SC_LOGIN_INFO = 2;
constexpr char		SC_ADD_OBJECT = 3;
constexpr char		SC_REMOVE_OBJECT = 4;
constexpr char		SC_MOVE_OBJECT = 5;
constexpr char		SC_CHAT = 6;
constexpr char		SC_LOGIN_OK = 7;
constexpr char		SC_LOGIN_FAIL = 8;
constexpr char		SC_STAT_CHANGE = 9;


// 오브젝트들의 외형을 결정하는 시리얼 넘버이다.
// 여러 오브젝트들이 같은 시리얼을 가질 수도 있다.
namespace Serial {
	namespace Character {
		constexpr int HAKUREI_REIMU = 1;
		constexpr int KONPAKU_YOUMU = 2;
		constexpr int PATCHOULI_KNOWLEDGE = 3;

		constexpr int END = 1000;
	};

	namespace NPC {
		constexpr int NPC_A = Character::END;
		constexpr int NPC_B = Character::END + 1;
		constexpr int NPC_C = Character::END + 2;

		constexpr int END = Character::END + 1000;
	}

	namespace Monster {
		constexpr int START = NPC::END;
		constexpr int SHROOM = START;
		constexpr int MUSHROOM = START + 1;
		constexpr int RIBBONPIG = START + 2;
		constexpr int COUNT = START + 3;

		constexpr int END = NPC::END + 1000;
	};
}

namespace Type
{
	namespace Wait {
		constexpr int FIXED = 1;
		constexpr int ROAMING = 2;
	}

	namespace Move {
		constexpr int PIECE = 1;
		constexpr int AGRO = 2;
	}
}

enum AnimationState {
	Idle,
	Walk,
	Attack,
	Die,
	Count
};

namespace TileInfo {
	constexpr int UNDEFINED_NONBLOCK	= 0x01;
	constexpr int UNDEFINED_BLOCK		= 0x02;
	constexpr int HENESYS_NONBLOCK		= 0x04;
	constexpr int HENESYS_BLOCK			= 0x08;

	constexpr int BLOCKING = UNDEFINED_BLOCK | HENESYS_BLOCK;
	constexpr int NONBLOCKING = UNDEFINED_NONBLOCK | HENESYS_NONBLOCK;

	constexpr int HENESYS = HENESYS_NONBLOCK | HENESYS_BLOCK;
};

namespace VillageBorder {
	using namespace std;

	constexpr pair<int, int> HENESYS_START				= { 900, 900 };
	constexpr pair<int, int> HENESYS_END				= { 1100, 1100 };
	constexpr pair<int, int> HENESYS_VILLAGE_START		= { 970, 970 };
	constexpr pair<int, int> HENESYS_VILLAGE_END		= { 1030, 1030 };

	constexpr pair<int, int> PERION_START				= { 700, 700 };
	constexpr pair<int, int> PERION_END					= { 900, 900 };
	constexpr pair<int, int> PERION_VILLAGE_START		= { 770, 770 };
	constexpr pair<int, int> PERION_VILLAGE_END			= { 830, 830 };

	constexpr bool InVillage(const pair<int, int>& position) {
		if (position.first >= PERION_VILLAGE_START.first && position.first < PERION_VILLAGE_START.first &&
			position.second >= PERION_VILLAGE_START.second && position.second < PERION_VILLAGE_START.second) {
			return true;
		}
		if (position.first >= PERION_VILLAGE_START.first && position.first < PERION_VILLAGE_START.first &&
			position.second >= PERION_VILLAGE_START.second && position.second < PERION_VILLAGE_START.second) {
			return true;
		}
		return false;
	}
};

#pragma pack(push, 1)

struct Short2 {
	short x, y;

	Short2() : x(0), y(0) {}
	Short2(short x, short y) : x(x), y(y) {}
	Short2(const Short2& s) : x(s.x), y(s.y) {}

	Short2& operator=(const Short2& rhs) { x = rhs.x, y = rhs.y; return *this; }
	Short2 operator+(const Short2& rhs) const { return Short2{ x + rhs.x, y + rhs.y }; }
	Short2 operator-(const Short2& rhs) const { return Short2{ x - rhs.x, y - rhs.y }; }
	Short2& operator+=(const Short2& rhs) { (*this) = (*this) + rhs; return *this; }
	Short2& operator-=(const Short2& rhs) { (*this) = (*this) - rhs; return *this; }
	bool operator==(const Short2& rhs) const { return (x == rhs.x && y == rhs.y);}

	bool operator<(const Short2& rhs) const {
		if (x != rhs.x) return x < rhs.x;
		return y < rhs.y;
	}
	friend struct std::hash<Short2>;
};

namespace std {
template <>
struct hash<Short2> {
	size_t operator()(const Short2& s) const {
		hash<short> hash_func;
		return (hash_func(s.x)) ^ (hash_func(s.y));
	}
};
}

struct PACKET
{
	unsigned short size;
	char type;
};

/*
 *  Client to Server
 */

struct CS_LOGIN_PACKET
{
	unsigned short size;
	char type;
	char name[NAME_SIZE];
	char id[ID_SIZE];
	char password[PASSWORD_SIZE];
};

struct CS_MOVE_PACKET
{
	unsigned short size;
	char type;
	unsigned char direction;
	unsigned moveTime;
};

struct CS_CHAT_PACKET {
	unsigned short size;			// 크기가 가변이다, mess가 작으면 size도 줄이자.
	char	type;
	char	mess[CHAT_SIZE];
};

struct CS_TELEPORT_PACKET {
	unsigned short size;
	char	type;
};

struct CS_LOGOUT_PACKET
{
	unsigned short size;
	char type;
};

/*
 *  Server to Client
 */

struct SC_LOGIN_INFO_PACKET 
{
	unsigned short size;
	char	type;
	unsigned int id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	Short2 coord;
};

struct SC_ADD_OBJECT_PACKET
{
	unsigned short size;
	char type;
	unsigned int id;
	int serial;
	Short2 coord;
	char name[NAME_SIZE];
	int level;
	int hp;
	int maxHp;
};

struct SC_REMOVE_OBJECT_PACKET
{
	unsigned short size;
	char type;
	unsigned int id;
};

struct SC_MOVE_OBJECT_PACKET
{
	unsigned short size;
	char type;
	unsigned int id;
	Short2 coord;
	UINT move_time;
};

struct SC_CHAT_PACKET
{
	unsigned short size;
	char type;
	unsigned int id;
	char message[CHAT_SIZE];	// 가변 패킷 크기
};

struct SC_LOGIN_OK_PACKET
{
	unsigned short size;
	char type;
};

struct SC_LOGIN_FAIL_PACKET
{
	unsigned short size;
	char type;
};

struct SC_STAT_CHANGE_PACKET 
{
	unsigned short size;
	char	type;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
};

#pragma pack(pop)
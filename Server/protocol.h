#pragma once
#include <windows.h>

#define NETWORK_DEBUG

constexpr int		PORT_NUM = 4000;
constexpr int		NAME_SIZE = 20;
constexpr int		CHAT_SIZE = 200;
constexpr int		BUF_SIZE = 1024;

constexpr int		ID_SIZE = 20;
constexpr int		PASSWORD_SIZE = 20;

constexpr int		MAX_USER = 20000;
constexpr int		MAX_NPC = 2000;

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
constexpr char		SC_LOGIN_FAIL = 2;

enum CharacterInfo {
	HAKUREI_REIMU,
	KONPAKU_YOUMU,
	PATCHOULI_KNOWLEDGE
};

enum AnimationState {
	Idle,
	Walk,
	Attack,
	Die,
	Count
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

	bool operator<(const Short2& rhs) const {
		if (x != rhs.x) return x < rhs.x;
		return y < rhs.y;
	}
};

struct packet
{
	unsigned char size;
	unsigned char type;
};

/*
 *  Client to Server
 */

struct cs_packet_login : public packet
{
	char name[NAME_SIZE];
	char id[ID_SIZE];
	char password[PASSWORD_SIZE];
};

struct cs_packet_move : public packet
{
	unsigned char direction;
	UINT moveTime;
};

struct cs_packet_logout : public packet
{
	unsigned int id;
};

/*
 *  Server to Client
 */

struct sc_packet_login_confirm : public packet
{
	unsigned int id;
};

struct sc_packet_login_fail : public packet
{
};

struct sc_packet_add_player : public packet
{
	unsigned int id;
	Short2 coord;
	char name[NAME_SIZE];
};

struct sc_packet_object_info : public packet
{
	unsigned int id;
	Short2 coord;
	UINT moveTime;
};

struct sc_packet_chat : public packet
{
	unsigned int id;
	char message[CHAT_SIZE];	// 가변 패킷 크기
};

struct sc_packet_exit_player : public packet
{
	unsigned int id;
};

#pragma pack(pop)
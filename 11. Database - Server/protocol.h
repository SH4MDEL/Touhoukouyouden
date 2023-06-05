#pragma once

#include <windows.h>

constexpr short		SERVER_PORT = 4000;
constexpr int		BUFSIZE = 1024;

constexpr int		NAMESIZE = 20;
constexpr int		IDSIZE = 20;
constexpr int		PASSWORDSIZE = 20;
constexpr int		CHATSIZE = 100;

constexpr int		MAP_HEIGHT = 2000;
constexpr int		MAP_WIDTH = 2000;

constexpr char		CS_PACKET_LOGIN = 1;
constexpr char		CS_PACKET_MOVE = 2;
constexpr char		CS_PACKET_LOGOUT = 100;

constexpr char		SC_PACKET_LOGIN_CONFIRM = 1;
constexpr char		SC_PACKET_LOGIN_FAIL = 2;
constexpr char		SC_PACKET_ADD_PLAYER = 3;
constexpr char		SC_PACKET_OBJECT_INFO = 4;
constexpr char		SC_PACKET_CHAT = 5;
constexpr char		SC_PACKET_EXIT_PLAYER = 6;

#define NETWORK_DEBUG

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
	char name[NAMESIZE];
	char id[IDSIZE];
	char password[PASSWORDSIZE];
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
	char name[NAMESIZE];
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
	char message[CHATSIZE];	// 가변 패킷 크기
};

struct sc_packet_exit_player : public packet
{
	unsigned int id;
};

#pragma pack(pop)
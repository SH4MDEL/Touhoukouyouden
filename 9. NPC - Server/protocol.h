#pragma once

#include <windows.h>

constexpr short		SERVER_PORT = 4000;
constexpr int		BUFSIZE = 1024;
constexpr int		NAMESIZE = 20;

constexpr int		MAP_HEIGHT = 2000;
constexpr int		MAP_WIDTH = 2000;

constexpr char		CS_PACKET_LOGIN = 1;
constexpr char		CS_PACKET_MOVE = 2;
constexpr char		CS_PACKET_LOGOUT = 100;

constexpr char		SC_PACKET_LOGIN_CONFIRM = 1;
constexpr char		SC_PACKET_ADD_PLAYER = 2;
constexpr char		SC_PACKET_OBJECT_INFO = 3;
constexpr char		SC_PACKET_EXIT_PLAYER = 4;

//#define NETWORK_DEBUG

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

// Client to Server

struct cs_packet_login : public packet
{
	char name[NAMESIZE];
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

// Server to Client

struct sc_packet_login_confirm : public packet
{
	unsigned int id;
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

struct sc_packet_exit_player : public packet
{
	unsigned int id;
};

#pragma pack(pop)
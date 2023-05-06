#pragma once

#include <windows.h>

constexpr short		SERVER_PORT = 4000;
constexpr int		BUFSIZE = 256;
constexpr int		NAMESIZE = 20;

constexpr int		MAP_HEIGHT = 400;
constexpr int		MAP_WIDTH = 400;

constexpr char		CS_PACKET_LOGIN = 1;
constexpr char		CS_PACKET_MOVE = 2;
constexpr char		CS_PACKET_LOGOUT = 100;

constexpr char		SC_PACKET_LOGIN_CONFIRM = 1;
constexpr char		SC_PACKET_ADD_PLAYER = 2;
constexpr char		SC_PACKET_OBJECT_INFO = 3;
constexpr char		SC_PACKET_EXIT_PLAYER = 4;

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
};

#define NETWORK_DEBUG

#pragma pack(push, 1)
struct packet
{
	unsigned char size;
	unsigned char type;
};

// Client to Server

struct cs_packet_login : public packet
{

};

struct cs_packet_move : public packet
{
	unsigned char id;
	Short2 coord;
};

struct cs_packet_logout : public packet
{
	unsigned char id;
};

// Server to Client

struct sc_packet_login_confirm : public packet
{
	unsigned char id;
};

struct sc_packet_add_player : public packet
{
	unsigned char id;
	Short2 coord;
};

struct sc_packet_object_info : public packet
{
	unsigned char id;
	Short2 coord;
};

struct sc_packet_exit_player : public packet
{
	unsigned char id;
};
#pragma pack(pop)
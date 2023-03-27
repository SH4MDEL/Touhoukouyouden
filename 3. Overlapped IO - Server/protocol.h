#pragma once
#pragma pack(push, 1)

#include <windows.h>

constexpr short		SERVER_PORT = 4000;
constexpr int		BUFSIZE = 256;

constexpr char		CS_PACKET_LOGIN = 1;
constexpr char		CS_PACKET_MOVE = 2;
constexpr char		CS_PACKET_LOGOUT = 100;

constexpr char		SC_PACKET_LOGIN_CONFIRM = 1;
constexpr char		SC_PACKET_ADD_PLAYER = 2;
constexpr char		SC_PACKET_OBJECT_INFO = 3;
constexpr char		SC_PACKET_EXIT_PLAYER = 4;

#define NETWORK_DEBUG

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
	POINT coord;
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
	POINT coord;
};

struct sc_packet_object_info : public packet
{
	unsigned char id;
	POINT coord;
};

struct sc_packet_exit_player : public packet
{
	unsigned char id;
};
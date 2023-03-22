#pragma once
#pragma pack(1)

#include <windows.h>

constexpr short		SERVER_PORT = 4000;
constexpr int		BUFSIZE = 256;

constexpr char		CS_PACKET_LOGIN = 1;
constexpr char		CS_PACKET_MOVE = 2;
constexpr char		CS_PACKET_LOGOUT = 100;

constexpr char		SC_PACKET_LOGIN_CONFIRM = 1;
constexpr char		SC_PACKET_OBJECT_INFO = 2;

struct packet
{
	unsigned char size;
	unsigned char type;
};

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


struct sc_packet_login_confirm : public packet
{
	unsigned char id;
};

struct sc_packet_object_info : public packet
{
	unsigned char id;
	POINT coord;
};
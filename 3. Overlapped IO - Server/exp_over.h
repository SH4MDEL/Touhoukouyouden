#pragma once
#include "stdafx.h"

class EXP_OVER
{
public:
	EXP_OVER(unsigned long long s_id, char num_bytes, const char* mess);
	~EXP_OVER() = default;

public:
	WSAOVERLAPPED m_wsaOver;
	unsigned long long m_sid;
	WSABUF m_wsaBuf;
	char m_sendMsg[BUFSIZE];

};


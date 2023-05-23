#pragma once
#include "stdafx.h"

// 확장 OVERLAPPED 구조체. 
// WSAOVERLAPPED 구조체에 더해 필요한 정보를 추가로 선언한다.
class EXP_OVER
{
public:
	EXP_OVER();
	EXP_OVER(char* packet);
	~EXP_OVER() = default;

public:
	WSAOVERLAPPED m_overlapped;
	WSABUF m_wsaBuf;
	char m_sendMsg[BUFSIZE];
	COMP_TYPE m_compType;
};


#pragma once
#include "stdafx.h"

// Ȯ�� OVERLAPPED ����ü. 
// WSAOVERLAPPED ����ü�� ���� �ʿ��� ������ �߰��� �����Ѵ�.
class EXPOVERLAPPED
{
public:
	EXPOVERLAPPED();
	EXPOVERLAPPED(char* packet);
	~EXPOVERLAPPED() = default;

public:
	WSAOVERLAPPED m_overlapped;
	WSABUF m_wsaBuf;
	char m_sendMsg[BUF_SIZE];
	COMP_TYPE m_compType;
};
#pragma once
#include "stdafx.h"
#include "exp_over.h"
#include "main.h"

class SESSION
{
public:
	SESSION();
	SESSION(unsigned long long id, SOCKET socket);
	~SESSION();

	void DoRecv();
	void DoSend(unsigned long long senderID, int numBytes, const char* buff);

	char m_recvBuf[BUFSIZE];

private:
	unsigned long long	m_id;

	WSABUF				m_recvWsabuf;
	WSABUF				m_sendWsabuf;

	WSAOVERLAPPED		m_recvOver;
	SOCKET				m_socket;
};


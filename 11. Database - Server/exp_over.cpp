#include "exp_over.h"

EXP_OVER::EXP_OVER()
{
	m_wsaBuf.len = BUFSIZE;
	m_wsaBuf.buf = m_sendMsg;
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	m_compType = COMP_TYPE::OP_RECV;
}

EXP_OVER::EXP_OVER(char* packet)
{
	m_wsaBuf.len = packet[0];
	m_wsaBuf.buf = m_sendMsg;
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	m_compType = COMP_TYPE::OP_SEND;
	memcpy(m_sendMsg, packet, packet[0]);
}

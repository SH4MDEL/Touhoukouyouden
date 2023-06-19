#include "expOverlapped.h"

EXPOVERLAPPED::EXPOVERLAPPED()
{
	m_wsaBuf.len = BUF_SIZE;
	m_wsaBuf.buf = m_sendMsg;
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	m_compType = COMP_TYPE::OP_RECV;
}

EXPOVERLAPPED::EXPOVERLAPPED(char* packet)
{
	m_wsaBuf.len = reinterpret_cast<short*>(packet)[0];
	m_wsaBuf.buf = m_sendMsg;
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	m_compType = COMP_TYPE::OP_SEND;
	memcpy(m_sendMsg, packet, m_wsaBuf.len);
}

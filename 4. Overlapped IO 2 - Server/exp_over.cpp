#include "exp_over.h"

EXP_OVER::EXP_OVER(unsigned long long sid, char num_bytes, const char* mess) :
	m_sid{ sid }
{
	ZeroMemory(&m_wsaOver, sizeof(m_wsaOver));
	m_wsaBuf.buf = m_sendMsg;
	m_wsaBuf.len = num_bytes /*+ 2*/;

	memcpy(m_sendMsg, mess, num_bytes);
	//m_sendMsg[0] = num_bytes + 2;
	//m_sendMsg[1] = static_cast<char>(sid);
}

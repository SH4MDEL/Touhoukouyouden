#include "session.h"

SESSION::SESSION()
{
	cout << "Unexpected Constructor Call Error!" << endl;
	exit(-1);
}

SESSION::SESSION(unsigned long long id, SOCKET socket) :
	m_id{ id }, m_socket{ socket }
{
	m_recvWsabuf.buf = m_recvBuf;
	m_recvWsabuf.len = BUFSIZE;
	m_sendWsabuf.buf = m_recvBuf;
	m_sendWsabuf.len = 0;
}

SESSION::~SESSION() { closesocket(m_socket); }

void SESSION::DoRecv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&m_recvOver, sizeof(m_recvOver));
	m_recvOver.hEvent = reinterpret_cast<HANDLE>(m_id);
	WSARecv(m_socket, &m_recvWsabuf, 1, 0, &recv_flag, &m_recvOver, RecvCallback);
}

void SESSION::DoSend(unsigned long long senderID, int numBytes, const char* buff)
{
	EXP_OVER* send_over = new EXP_OVER(senderID, numBytes, buff);
	WSASend(m_socket, &send_over->m_wsaBuf, 1, 0, 0, &send_over->m_wsaOver, SendCallback);
}

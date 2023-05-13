#include "client.h"

CLIENT::CLIENT() : 
	m_state{ State::FREE }, m_id{ -1 }, m_lastMoveTime{ 0 }, m_socket { 0 }, 
	m_position{ 0, 0 }, m_prevRemain{ 0 }
{}

CLIENT::~CLIENT() { closesocket(m_socket); }

void CLIENT::DoRecv()
{
	DWORD recv_flag = 0;
	memset(&m_recvOver.m_overlapped, 0, sizeof(m_recvOver.m_overlapped));
	m_recvOver.m_wsaBuf.len = BUFSIZE - m_prevRemain;

	m_recvOver.m_wsaBuf.buf = m_recvOver.m_sendMsg + m_prevRemain;
	int retval = WSARecv(m_socket, &m_recvOver.m_wsaBuf, 1, 0, &recv_flag, 
		&m_recvOver.m_overlapped, 0);
}

void CLIENT::DoSend(void* packet)
{
	EXP_OVER* send_over = new EXP_OVER(reinterpret_cast<char*>(packet));
	WSASend(m_socket, &send_over->m_wsaBuf, 1, 0, 0, &send_over->m_overlapped, 0);
}

void CLIENT::SendLoginConfirm()
{
}

void CLIENT::SendAddPlayer(INT id)
{
	sc_packet_add_player sendpk;
	sendpk.size = sizeof(sc_packet_add_player);
	sendpk.type = SC_PACKET_ADD_PLAYER;
	sendpk.id = id;
	sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_PACKET_ADD_PLAYER ¼Û½Å - ID : " << (int)sendpk.id << endl;
#endif
}

void CLIENT::SendObjectInfo(INT id)
{

}

void CLIENT::SendExitPlayer(INT id)
{
}

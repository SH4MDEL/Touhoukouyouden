#include "client.h"

CLIENT::CLIENT() : OBJECT(), m_socket{0}
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
	// id를 가진 플레이어를 새로 생성한다.
	m_viewLock.lock();
	if (m_viewList.count(id)) {
		// 해당 id를 가진 플레이어가 시야에 이미 존재한다.
		m_viewLock.unlock();
		// 이동만 시키고 종료한다.
		SendObjectInfo(id);
		return;
	}
	m_viewList.insert(id);
	m_viewLock.unlock();

	sc_packet_add_player sendpk;
	sendpk.size = sizeof(sc_packet_add_player);
	sendpk.type = SC_PACKET_ADD_PLAYER;
	sendpk.id = id;
	if (id < MAX_USER) {
		sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
	}
	else {
		sendpk.coord = g_gameServer.GetNPC((UINT)id)->m_position;
	}
	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_PACKET_ADD_PLAYER 송신 - ID : " << id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendObjectInfo(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// 시야에 해당 오브젝트가 존재하지 않는다면
		m_viewLock.unlock();
		// 그 오브젝트를 먼저 추가해준다.
		SendAddPlayer(id);
		return;
	}
	m_viewLock.unlock();

	sc_packet_object_info sendpk;
	sendpk.size = sizeof(sc_packet_object_info);
	sendpk.type = SC_PACKET_OBJECT_INFO;
	sendpk.id = id;
	if (id < MAX_USER) {
		sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
		sendpk.moveTime = g_gameServer.GetClient(id)->m_lastMoveTime;
	}
	else {
		sendpk.coord = g_gameServer.GetNPC((UINT)id)->m_position;
		sendpk.moveTime = 0;
	}

	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_PACKET_OBJECT_INFO 송신 - ID : " << id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendExitPlayer(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// 이미 시야에 해당 오브젝트가 존재하지 않는다면
		m_viewLock.unlock();
		// 아무 일도 하지 않는다.
		return;
	}
	m_viewList.erase(id);
	m_viewLock.unlock();

	sc_packet_exit_player packet;
	packet.size = sizeof(sc_packet_exit_player);
	packet.type = SC_PACKET_EXIT_PLAYER;
	packet.id = id;
	DoSend(&packet);

#ifdef NETWORK_DEBUG
	cout << "SC_PACKET_EXIT_PLAYER 송신 - ID : " << id << endl;
#endif
}

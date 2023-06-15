#include "client.h"

CLIENT::CLIENT() : OBJECT(), m_socket{0}
{}

CLIENT::~CLIENT() { closesocket(m_socket); }

void CLIENT::DoRecv()
{
	DWORD recv_flag = 0;
	memset(&m_recvOver.m_overlapped, 0, sizeof(m_recvOver.m_overlapped));
	m_recvOver.m_wsaBuf.len = BUF_SIZE - m_prevRemain;

	m_recvOver.m_wsaBuf.buf = m_recvOver.m_sendMsg + m_prevRemain;
	int retval = WSARecv(m_socket, &m_recvOver.m_wsaBuf, 1, 0, &recv_flag, 
		&m_recvOver.m_overlapped, 0);
}

void CLIENT::DoSend(void* packet)
{
	EXPOVERLAPPED* send_over = new EXPOVERLAPPED(reinterpret_cast<char*>(packet));
	WSASend(m_socket, &send_over->m_wsaBuf, 1, 0, 0, &send_over->m_overlapped, 0);
}

void CLIENT::SendLoginConfirm()
{
}

void CLIENT::SendAddPlayer(INT id)
{
	// id�� ���� �÷��̾ ���� �����Ѵ�.
	m_viewLock.lock();
	if (m_viewList.count(id)) {
		// �ش� id�� ���� �÷��̾ �þ߿� �̹� �����Ѵ�.
		m_viewLock.unlock();
		// �̵��� ��Ű�� �����Ѵ�.
		SendObjectInfo(id);
		return;
	}
	m_viewList.insert(id);
	m_viewLock.unlock();

	SC_ADD_OBJECT_PACKET sendpk;
	sendpk.size = sizeof(SC_ADD_OBJECT_PACKET);
	sendpk.type = SC_ADD_OBJECT;
	sendpk.id = id;
	if (id < MAX_USER) {
		sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
	}
	else {
		sendpk.coord = g_gameServer.GetNPC((UINT)id)->m_position;
	}
	strcpy_s(sendpk.name, g_gameServer.GetClient((UINT)id)->m_name);
	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_ADD_OBJECT �۽� - ID : " << m_id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendObjectInfo(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// �þ߿� �ش� ������Ʈ�� �������� �ʴ´ٸ�
		m_viewLock.unlock();
		// �� ������Ʈ�� ���� �߰����ش�.
		SendAddPlayer(id);
		return;
	}
	m_viewLock.unlock();

	SC_MOVE_OBJECT_PACKET sendpk;
	sendpk.size = sizeof(SC_MOVE_OBJECT_PACKET);
	sendpk.type = SC_MOVE_OBJECT;
	sendpk.id = id;
	if (id < MAX_USER) {
		sendpk.coord = g_gameServer.GetClient((UINT)id)->m_position;
		sendpk.move_time = g_gameServer.GetClient(id)->m_lastMoveTime;
	}
	else {
		sendpk.coord = g_gameServer.GetNPC((UINT)id)->m_position;
		sendpk.move_time = 0;
	}

	DoSend(&sendpk);
#ifdef NETWORK_DEBUG
	cout << "SC_MOVE_OBJECT �۽� - ID : " << m_id << ", x : " << sendpk.coord.x << ", y : " << sendpk.coord.y << endl;
#endif
}

void CLIENT::SendChat(INT id, const char* message)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// �þ߿� �ش� ������Ʈ�� �������� �ʴ´ٸ�
		m_viewLock.unlock();
		// �� ������Ʈ�� ���� �߰����ش�.
		SendAddPlayer(id);
		return;
	}
	m_viewLock.unlock();

	SC_CHAT_PACKET packet;
	packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
	packet.type = SC_CHAT;
	packet.id = id;
	strcpy_s(packet.message, message);
	DoSend(&packet);

#ifdef NETWORK_DEBUG
	cout << "SC_CHAT �۽� - ID : " << m_id;
#endif
}

void CLIENT::SendExitPlayer(INT id)
{
	m_viewLock.lock();
	if (!m_viewList.count(id)) {
		// �̹� �þ߿� �ش� ������Ʈ�� �������� �ʴ´ٸ�
		m_viewLock.unlock();
		// �ƹ� �ϵ� ���� �ʴ´�.
		return;
	}
	m_viewList.erase(id);
	m_viewLock.unlock();

	SC_REMOVE_OBJECT_PACKET packet;
	packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	packet.type = SC_REMOVE_OBJECT;
	packet.id = id;
	DoSend(&packet);

#ifdef NETWORK_DEBUG
	cout << "SC_REMOVE_OBJECT �۽� - ID : " << m_id << endl;
#endif
}

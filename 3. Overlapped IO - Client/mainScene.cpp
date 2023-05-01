#include "mainScene.h"

MainScene::MainScene(Tag tag) : Scene(tag), m_windowSize{ ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) }
{
}

void MainScene::OnCreate()
{
}

void MainScene::BuildObjects()
{
	m_board = make_shared<Board>(POINT{ m_windowSize.x / 2, m_windowSize.y / 2 }, POINT{ m_windowSize.y, m_windowSize.y });
	m_board->SetImage(L"..\\Resource\\Chessboard.png");

	InitServer();
}

void MainScene::OnDestroy()
{

}

void MainScene::Update(float timeElapsed)
{
	Recv();
}

void MainScene::Render(HDC hdc)
{
	m_board->Render(hdc);
}

void MainScene::AddPlayer(int id, POINT position)
{
	auto player = make_shared<Piece>(position, POINT{ m_windowSize.y / 8, m_windowSize.y / 8 });
	player->SetImage(L"..\\Resource\\Piece.png");
	player->SetBoard(m_board);
	m_board->SetPlayer(id, player);

	m_players.insert({ id, player });
}

void MainScene::ExitPlayer(int id)
{
	if (m_players.find(id) != m_players.end()) {
		m_players.erase(id);
	}
	m_board->ExitPlayer(id);
}

void MainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID)
	{
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_DOWN:
		case VK_RIGHT:
		case VK_UP:
		case VK_LEFT:
			cs_packet_move packet;
			packet.size = sizeof(cs_packet_move);
			packet.type = CS_PACKET_MOVE;
			packet.id = g_playerID;
			packet.coord.x = Move::dx[VK_DOWN - wParam];
			packet.coord.y = Move::dy[VK_DOWN - wParam];
			Send(&packet);
#ifdef NETWORK_DEBUG
			cout << "CS_PACKET_MOVE ¼Û½Å" << endl;
#endif
			break;
		}

	}

}

#include "mainScene.h"

MainScene::MainScene(Tag tag) : Scene(tag), m_windowSize{ ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) }
{
}

void MainScene::OnCreate()
{
}

void MainScene::OnDestroy()
{

}

void MainScene::BuildObjects()
{
	m_board = make_shared<Board>(POINT{ m_windowSize.x / 2, m_windowSize.y / 2}, POINT{ m_windowSize.y, m_windowSize.y });
	m_board->SetImage(L"..\\Resource\\Chessboard.png");

	m_player = make_shared<Piece>(POINT{ 0, 0 }, POINT{ m_windowSize.y / 8, m_windowSize.y / 8 });
	m_player->SetImage(L"..\\Resource\\Piece.png");

	m_board->SetPiece(m_player);
	m_player->SetBoard(m_board);

	InitServer();
}

void MainScene::Update(float timeElapsed)
{
	Recv();
}

void MainScene::Render(HDC hdc)
{
	m_board->Render(hdc);
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

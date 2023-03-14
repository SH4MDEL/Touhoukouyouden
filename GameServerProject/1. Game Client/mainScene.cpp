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
	m_board->SetImage(L"Resource\\Chessboard.png");

	m_player = make_shared<Piece>(POINT{ 0, 0 }, POINT{ m_windowSize.y / 8, m_windowSize.y / 8 });
	m_player->SetImage(L"Resource\\Piece.png");

	m_board->SetPiece(m_player);
	m_player->SetBoard(m_board);
}

void MainScene::Update(float timeElapsed)
{
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
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_LEFT:
			m_board->Move(m_player->GetBoardPosition(), {
				m_player->GetBoardPosition().x + Move::dx[0],
				m_player->GetBoardPosition().y + Move::dy[0] });
			break;
		case VK_RIGHT:
			m_board->Move(m_player->GetBoardPosition(), {
				m_player->GetBoardPosition().x + Move::dx[1],
				m_player->GetBoardPosition().y + Move::dy[1] });
			break;
		case VK_UP:
			m_board->Move(m_player->GetBoardPosition(), {
				m_player->GetBoardPosition().x + Move::dx[2],
				m_player->GetBoardPosition().y + Move::dy[2] });
			break;
		case VK_DOWN:
			m_board->Move(m_player->GetBoardPosition(), {
				m_player->GetBoardPosition().x + Move::dx[3],
				m_player->GetBoardPosition().y + Move::dy[3] });
			break;
		}

	}

}

#pragma once
#include "scene.h"
#include "object.h"
#include "board.h"
#include "piece.h"

class MainScene : public Scene
{
public:
	MainScene() = default;
	MainScene(Tag tag);
	virtual ~MainScene() = default;

	void OnCreate() override;
	void OnDestroy() override;

	void BuildObjects() override;
	void Update(float timeElapsed) override;
	void Render(HDC hdc) override;

	void OnProcessingKeyboardMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam) override;

private:
	POINT				m_windowSize;
	shared_ptr<Board>	m_board;
	shared_ptr<Piece>	m_player;
};
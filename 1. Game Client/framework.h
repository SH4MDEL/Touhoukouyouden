#include "stdafx.h"
#include "timer.h"
#include "scene.h"

class Framework
{
public:
	Framework();
	~Framework() = default;

	bool OnCreate(HINSTANCE hInstance, HWND hWnd, const RECT& rc);
	bool OnDestroy(); 

	void CreatebackBuffer();	// HBITMAP을 만든다.
	void BuildObjects();

	void FrameAdvance();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool OnProcessingKeyboardMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingMouseMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
	HRESULT OnProcessingWindowMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);

	void Update(FLOAT timeElapsed);
	void Render();

	void SetBKColor(COLORREF color);

private:
	HINSTANCE										m_hInstance;

	HWND											m_hWnd;
	RECT											m_rcClient;	// 클라이언트 크기

	HBITMAP											m_hBitmapBackBuffer;	// 비트맵 핸들.
	HDC												m_hDC;				// 윈도우 핸들

	COLORREF										m_clrBackBuffer;
	HBRUSH											m_hbrBackground;

	TCHAR											m_captionTitle[MAX_TITLE];
	INT												m_titleLength;

	Timer											m_timer;

	array<unique_ptr<Scene>, Scene::Tag::Count>		m_scenes;
	INT												m_sceneIndex;
};
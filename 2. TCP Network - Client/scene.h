#pragma once
#include "stdafx.h"
#include "2. TCP Network - Client.h"

class Scene abstract
{
public:
	enum Tag {
		Main,
		Count
	};

	Scene() = default;
	Scene(Tag tag);
	virtual ~Scene() = default;

	virtual void OnCreate() = 0;
	virtual void OnDestroy() = 0;

	virtual void BuildObjects() = 0;
	virtual void Update(float timeElapsed) = 0;
	virtual void Render(HDC hdc) = 0;

	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam) = 0;

protected:
	Tag		m_tag;
};


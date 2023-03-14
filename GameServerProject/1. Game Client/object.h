#pragma once
#include "stdafx.h"

class Object
{
public:
	Object() = default;
	Object(POINT position, POINT length);
	virtual ~Object() = default;

	virtual void Update(float timeElapsed);
	virtual void Render(HDC hdc);
	virtual void Render(HDC hdc, POINT position);

	virtual void SetImage(wstring fileName);
	virtual void SetPosition(POINT position) { m_position = position; }
	virtual void SetLength(POINT length) { m_length = length; }

protected:
	POINT m_position;
	POINT m_length;

	CImage m_image;
};


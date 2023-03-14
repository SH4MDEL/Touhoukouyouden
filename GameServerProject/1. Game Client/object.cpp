#include "Object.h"

Object::Object(POINT position, POINT length) : m_position(position), m_length(length) {}

void Object::Update(float timeElapsed) {}

void Object::Render(HDC hdc)
{
	m_image.Draw(hdc, m_position.x - m_length.x / 2, m_position.y - m_length.y / 2,
		m_length.x, m_length.y);
}

void Object::Render(HDC hdc, POINT position)
{
	m_image.Draw(hdc, position.x, position.y,
		m_length.x, m_length.y);
}

void Object::SetImage(wstring fileName)
{
	m_image.Load(fileName.c_str());
}

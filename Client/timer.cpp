#include "timer.h"

Timer::Timer() : m_tick{}, m_deltaTime{ 0.f }, m_keydown{}, m_keyDeltaTime{ 0.f }
{
	QueryPerformanceFrequency(&m_frequency);
}

void Timer::Tick()
{
	LARGE_INTEGER temp;
	QueryPerformanceCounter(&temp);
	m_deltaTime = (temp.QuadPart - m_tick.QuadPart) / static_cast<FLOAT>(m_frequency.QuadPart);
	m_tick = temp;
}

void Timer::Keydown()
{
	LARGE_INTEGER temp;
	QueryPerformanceCounter(&temp);
	m_keyDeltaTime = (temp.QuadPart - m_keydown.QuadPart) / static_cast<FLOAT>(m_frequency.QuadPart);
	m_keydown = temp;
}
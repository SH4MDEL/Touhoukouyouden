#pragma once
#include "stdafx.h"
#include "singleton.h"

class Timer : public Singleton<Timer>
{
public:
	Timer();
	~Timer() = default;

	void Tick();
	void Keydown();
	FLOAT GetDeltaTime() const { return m_deltaTime; }
	FLOAT GetKeyDeltaTime() const { return m_keyDeltaTime; }
	FLOAT GetFPS() const { return 1.0f / m_deltaTime; }

private:
	LARGE_INTEGER	m_tick;
	LARGE_INTEGER	m_frequency;
	FLOAT			m_deltaTime;

	LARGE_INTEGER	m_keydown;
	FLOAT			m_keyDeltaTime;
};
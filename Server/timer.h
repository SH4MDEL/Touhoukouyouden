#pragma once
#include "stdafx.h"
#include "singleton.h"

struct TimerEvent
{
	enum Type { MOVE, ATTACK, HEAL, RESURRECTION };

	UINT m_id;
	Type m_type;
	chrono::system_clock::time_point m_executeTime;
	INT m_eventMsg;	// 나중에 삭제할거 생각하기
	UINT m_targetid;

	constexpr bool operator<(const TimerEvent& rhs) const {
		return m_executeTime > rhs.m_executeTime;
	}
};

class Timer : public Singleton<Timer>
{
public:
	Timer() = default;
	~Timer() = default;

	void TimerThread(HANDLE hiocp);
	void AddTimerEvent(UINT id, TimerEvent::Type type, chrono::system_clock::time_point executeTime, INT eventMsg, UINT targetid);
private:
	concurrency::concurrent_priority_queue<TimerEvent> m_timerQueue;
};


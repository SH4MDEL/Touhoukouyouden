#include "timer.h"
#include "expOverlapped.h"

void Timer::AddTimerEvent(UINT id, TimerEvent::Type type, chrono::system_clock::time_point executeTime, INT eventMsg, UINT targetid)
{
	m_timerQueue.push(TimerEvent{ id, type, executeTime, eventMsg, targetid });
}

void Timer::TimerThread(HANDLE hiocp)
{
	while (true)
	{
		auto currentTime = chrono::system_clock::now();
		TimerEvent ev;
		if (m_timerQueue.try_pop(ev)) {
			if (ev.m_executeTime > currentTime) {
				m_timerQueue.push(ev);
				this_thread::sleep_for(1ms);
				continue;
			}

			switch (ev.m_type)
			{
			case TimerEvent::FIXED:
			{
				EXPOVERLAPPED* over = new EXPOVERLAPPED;
				over->m_compType = COMP_TYPE::TIMER_NPC_FIXED;
				PostQueuedCompletionStatus(hiocp, 1, ev.m_id, &over->m_overlapped);
				break;
			}
			case TimerEvent::ROAMING:
			{
				EXPOVERLAPPED* over = new EXPOVERLAPPED;
				over->m_compType = COMP_TYPE::TIMER_NPC_ROAMING;
				PostQueuedCompletionStatus(hiocp, 1, ev.m_id, &over->m_overlapped);
				break;
			}
			}
			continue;
		}
		this_thread::sleep_for(1ms);
	}
}

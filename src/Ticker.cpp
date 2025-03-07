#include "Ticker.h"

namespace reminder
{
	Ticker& Ticker::instance()
	{
		static Ticker s_instance;
		return s_instance;
	}

	void Ticker::emitUpdateQueuesOnce()
	{
		emit updateHistoryQueues(m_scheduleManager.getHistoryQueue());
		emit updateCheckQueues(m_scheduleManager.getCheckQueue());
	}

	Ticker::Ticker()
		: m_timer(this)
	{
		std::array<std::queue<Schedule>, 2> schQueues;
		const auto result = m_scheduleManager.getNotifications(schQueues);
		if (result != ScheduleQueue::CheckStatus::NotCheckYet)
		{
			if (!schQueues[0].empty())
			{
				emit showOutOfDateNotification(schQueues[0]);
			}
			if (!schQueues[1].empty())
			{
				emit showNotification(schQueues[1]);
			}
			if (result == ScheduleQueue::CheckStatus::MatchOnceEvent)
			{
				emitUpdateQueuesOnce();
			}
		}

		connect(&m_timer, &QTimer::timeout, this, &Ticker::emitSignals);
		m_timer.start(100);
	}

	void Ticker::emitSignals()
	{
		emit updateClock();
		std::array<std::queue<Schedule>, 2> schQueues;
		const auto result = m_scheduleManager.getNotifications(schQueues);
		if (result != ScheduleQueue::CheckStatus::NotCheckYet)
		{
			if (!schQueues[0].empty())
			{
				emit showOutOfDateNotification(schQueues[0]);
			}
			if (!schQueues[1].empty())
			{
				emit showNotification(schQueues[1]);
			}
			if (result == ScheduleQueue::CheckStatus::MatchOnceEvent)
			{
				emitUpdateQueuesOnce();
			}
		}
	}
}
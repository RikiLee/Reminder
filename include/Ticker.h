#pragma once
#include <QTimer>
#include "Schedule.h"

namespace reminder
{
	class Ticker : public QObject
	{
		Q_OBJECT
	public:
		static Ticker& instance();

	public:
		void addTask(const Schedule& task) 
		{ 
			m_scheduleManager.addTask(task); 
			emit updateCheckQueues(m_scheduleManager.getCheckQueue()); 
		}
		void emitUpdateQueuesOnce();

		void removeTask(size_t id) 
		{ 
			m_scheduleManager.removeCommonSchedule(id); 
			emit updateCheckQueues(m_scheduleManager.getCheckQueue()); 
		}

		void updateTask(size_t id, const Schedule& sch)
		{
			m_scheduleManager.updateTask(id, sch);
			emit updateCheckQueues(m_scheduleManager.getCheckQueue());
		}

		void removeHistory(size_t id) 
		{ 
			m_scheduleManager.removeHistorySchedule(id); 
			emit updateHistoryQueues(m_scheduleManager.getHistoryQueue()); 
		}

		const Schedule& getCommonSchedule(size_t id) const { return m_scheduleManager.getCommonSchedule(id); }
		const Schedule& getHistorySchedule(size_t id) const { return m_scheduleManager.getCommonSchedule(id); }

	signals:
		void updateClock();
		void showOutOfDateNotification(std::queue<Schedule>& schedules);
		void showNotification(std::queue<Schedule>& schedules);

		void updateHistoryQueues(const std::vector<std::pair<Schedule, size_t>>& hqs);
		void updateCheckQueues(const std::vector<std::pair<Schedule, size_t>>& hqs);

	private:
		Ticker();
		Ticker(const Ticker& src) = delete;
		Ticker& operator=(Ticker& src) = delete;
		~Ticker() = default;

		QTimer m_timer;
		ScheduleQueue m_scheduleManager;

	private slots:
		void emitSignals();
	};
}
#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <array>

namespace reminder
{
    typedef std::chrono::system_clock::time_point TimePoint;

    enum class ScheduleType : uint8_t
    {
        Once,
        Loop
    };

    enum class LoopType : uint8_t
    {
        EveryYear,
        EveryMonth,
        EveryWeek,
        EveryDay,
        EveryHour,
        None
    };

    enum class Weekdays : uint8_t
    {
        Sunday = 0,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        None
    };

    enum class Order : int8_t
    {
        Order_Reverse_5 = -5,
        Order_Reverse_4 = -4,
        Order_Reverse_3 = -3,
        Order_Reverse_2 = -2,
        Order_Reverse_1 = -1,
        Order_Dont_Care = 0,
        Order_Normal_1 = 1,
        Order_Normal_2 = 2,
        Order_Normal_3 = 3,
        Order_Normal_4 = 4,
        Order_Normal_5 = 5
    };

    enum class TimeMatchResult : uint8_t
    {
        MATCH_FAILED,
        MATCH_SUCCEED,
        MATCH_OUT_OF_DATE
    };

    struct ScheduleTime
    {
        //static const uint16_t DoNotCheckYear = 0;
        //static const uint16_t EveryYearCheckOnce = 1000;

        //static const uint16_t DoNotCheckMonth = 0;
        //static const uint8_t EveryMonthCheckOnce = 13;

        static const uint8_t DoNotCheckDay = 0;
        //static const uint8_t EveryDayCheckOnce = 32;
        //static const uint8_t LastDayAtTheMonth = 33;

        //static const uint8_t EveryHourCheckOnce = 24;
        static const uint8_t DoNotCheckHour = 25;

        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = DoNotCheckDay;   // if day != DoNotCheckDay, then don't care weekday 
        Weekdays weekday = Weekdays::Sunday;
        uint8_t hour = DoNotCheckHour; // if hour == DoNotCheckHour, then don't care hour and minute
        uint8_t minute = 0;

        bool operator<(const ScheduleTime& sch) const noexcept
        {
            if (year != sch.year)
                return year < sch.year;
            if (month != sch.month)
                return month < sch.month;
            return true;
        }

        bool operator==(const ScheduleTime& sch) const noexcept
        {
            return year == sch.year && month == sch.month && day == sch.day
                && weekday == sch.weekday && hour == sch.hour && minute == sch.minute;
        }

    };

    enum class DisplayEffect : uint8_t
    {
        Text,
        Image,
        Audio,
        Video,
        WebSite
    };

    struct Schedule
    {
        std::string m_description;
        std::string m_event;
        ScheduleTime m_timePoint;
        std::string m_mediaURL;
        DisplayEffect m_displayEffect = DisplayEffect::Text;
        ScheduleType m_scheduleType = ScheduleType::Once; // if m_scheduleType == Once then don't care m_loopType
        LoopType m_loopType = LoopType::EveryYear;

        bool operator==(const Schedule& sch) const noexcept
        {
            return m_description == sch.m_description && m_event == sch.m_event 
                && m_timePoint == sch.m_timePoint 
                && m_mediaURL == sch.m_mediaURL && m_displayEffect == sch.m_displayEffect;
        }

        TimeMatchResult match(const std::tm& time) const;
    };

    class ScheduleQueue final
    {
    public:
        ScheduleQueue()
        {
            initQueues();
        }

        ~ScheduleQueue()
        {
            saveQueues();
        }

        void updateTask(size_t id, const Schedule& sch)
        {
            m_checkQueue[id] = sch;
        }

        void clearHistory() noexcept
        {
            for (size_t i = 0, sz = m_historyQueue.size(); i < sz; ++i)
            {
                m_removedHistoryQueue.emplace(i);
            }
        }

        void removeHistorySchedule(size_t i)
        {
            if (i < m_historyQueue.size())
            {
                m_removedHistoryQueue.emplace(i);
            }
        }

        void removeCommonSchedule(size_t i)
        {
            if (i < m_checkQueue.size())
            {
                m_removedCommonQueue.emplace(i);
            }
        }

        enum CheckStatus : uint8_t
        {
            NotCheckYet = 0,
            MatchButNoOnceEvent = 1,
            MatchOnceEvent = 2
        };
        CheckStatus getNotifications(std::array<std::queue<Schedule>, 2> &schQueues);

        void addTask(const Schedule& task)
        {
            m_checkQueue.push_back(task);
        }

        std::vector<std::pair<Schedule, size_t>> getHistoryQueue() const 
        {
            std::vector<std::pair<Schedule, size_t>> result;
            for (size_t i = 0, sz = m_historyQueue.size(); i < sz; ++i)
            {
                if (m_removedHistoryQueue.count(i) == 0)
                {
                    result.emplace_back(m_historyQueue[i], i);
                }
            }
            return result;
        }
        std::vector<std::pair<Schedule, size_t>> getCheckQueue() const
        {
            std::vector<std::pair<Schedule, size_t>> result;
            for (size_t i = 0, ls = m_checkQueue.size(); i < ls; ++i)
            {
                if ((m_notifiedCommonQueue.count(i) == 0) && (m_removedCommonQueue.count(i) == 0))
                {
                    result.emplace_back(m_checkQueue[i], i);
                }
            }
            return result;
        }


        const Schedule& getCommonSchedule(size_t id) const
        {
            return m_checkQueue.at(id);
        }
        const Schedule& getHistorySchedule(size_t id) const
        {
            return m_historyQueue.at(id);
        }

    private:
        // init queues at the first time
        void initQueues();
        // save Schedule list
        void saveQueues();

        static constexpr const char* m_fileName = "schedules.ini";

        typedef uint8_t NewTimeFlags;
        enum NewTimeBits : uint8_t
        {
            NewYear = 1,
            NewMonth = 2,
            NewWeek = 4,
            NewDay = 8,
            NewHour = 16,
            NewMinute = 32
        };

        void update(NewTimeFlags nt)
        {
            if (nt & NewYear) m_notifiedYearQueue.clear();
            if (nt & NewMonth) m_notifiedMonthQueue.clear();
            if (nt & NewWeek) m_notifiedWeekQueue.clear();
            if (nt & NewDay) m_notifiedDayQueue.clear();
            if (nt & NewHour) m_notifiedHourQueue.clear();
        }

        NewTimeFlags getNewTimeFlags(const std::tm& time) const noexcept
        {
            std::time_t lastTime = std::chrono::system_clock::to_time_t(m_lastUpdateTimePoint);
            std::tm lastLocalTime = *std::localtime(&lastTime);
            NewTimeFlags result = 0;
            if (time.tm_year != lastLocalTime.tm_year) result |= NewYear;
            if (time.tm_mon != lastLocalTime.tm_mon) result |= NewMonth;
            if (time.tm_wday != lastLocalTime.tm_wday) result |= NewWeek;
            if (time.tm_mday != lastLocalTime.tm_mday) result |= NewDay;
            if (time.tm_hour != lastLocalTime.tm_hour) result |= NewHour;
            if (time.tm_min != lastLocalTime.tm_min) result |= NewMinute;
            return result;
        }

        bool checked(size_t id) const noexcept
        {
            if (m_removedCommonQueue.count(id)) return true;
            if (m_notifiedCommonQueue.count(id)) return true;
            if (m_notifiedYearQueue.count(id)) return true;
            if (m_notifiedMonthQueue.count(id)) return true;
            if (m_notifiedWeekQueue.count(id)) return true;
            if (m_notifiedDayQueue.count(id)) return true;
            if (m_notifiedHourQueue.count(id)) return true;

            return false;
        }
        
    private:
        std::vector<Schedule> m_historyQueue;
        std::vector<Schedule> m_checkQueue;
        std::unordered_set<size_t> m_notifiedCommonQueue;

        std::unordered_set<size_t> m_notifiedYearQueue;
        std::unordered_set<size_t> m_notifiedMonthQueue;
        std::unordered_set<size_t> m_notifiedWeekQueue;
        std::unordered_set<size_t> m_notifiedDayQueue;
        std::unordered_set<size_t> m_notifiedHourQueue;

        std::unordered_set<size_t> m_removedCommonQueue;
        std::unordered_set<size_t> m_removedHistoryQueue;

        TimePoint m_lastUpdateTimePoint{};
        std::mutex m_mtx;
        bool m_firstRun = true;
    };

}
#include "Schedule.h"
#include <fstream>
#include <regex>
#include <algorithm>
#include <cassert>

namespace reminder
{
    TimeMatchResult Schedule::match(const std::tm& time) const
    {
        auto currentYear = time.tm_year + 1900;
        auto currentMonth = time.tm_mon + 1;
        auto currentMDay = time.tm_mday;
        auto currentWDay = static_cast<Weekdays>(time.tm_wday);
        auto currentHour = time.tm_hour;
        auto currentMinute = time.tm_min;
     
        if (m_scheduleType == ScheduleType::Once)
        {
            if (m_timePoint.year < currentYear)
                return TimeMatchResult::MATCH_OUT_OF_DATE;
            else if (m_timePoint.year > currentYear)
                return TimeMatchResult::MATCH_FAILED;

            if (m_timePoint.month < currentMonth)
                return TimeMatchResult::MATCH_OUT_OF_DATE;
            else if (m_timePoint.month > currentMonth)
                return TimeMatchResult::MATCH_FAILED;

            if (m_timePoint.day == ScheduleTime::DoNotCheckDay)
            {

            }
            else
            {
                if (m_timePoint.day < currentMDay)
                    return TimeMatchResult::MATCH_OUT_OF_DATE;
                else if (m_timePoint.day > currentMDay)
                    return TimeMatchResult::MATCH_FAILED;
            }
            
            if (m_timePoint.hour != ScheduleTime::DoNotCheckHour)
            {
                if (m_timePoint.hour < currentHour)
                    return TimeMatchResult::MATCH_OUT_OF_DATE;
                else if (m_timePoint.hour > currentHour)
                    return TimeMatchResult::MATCH_FAILED;

                if (m_timePoint.minute < currentMinute)
                    return TimeMatchResult::MATCH_OUT_OF_DATE;
                else if (m_timePoint.minute > currentMinute)
                    return TimeMatchResult::MATCH_FAILED;
            }
        }
        else
        {
            switch (m_loopType)
            {
            case reminder::LoopType::EveryYear:
                if (m_timePoint.month != currentMonth)
                    return TimeMatchResult::MATCH_FAILED;
                [[fallthrough]];
            case reminder::LoopType::EveryMonth:
                if (m_timePoint.day != currentMDay)
                    return TimeMatchResult::MATCH_FAILED;
                [[fallthrough]];
            case reminder::LoopType::EveryWeek:
                if (m_timePoint.weekday != currentWDay)
                    return TimeMatchResult::MATCH_FAILED;
                if (m_timePoint.hour == ScheduleTime::DoNotCheckHour)
                    break;
                [[fallthrough]];
            case reminder::LoopType::EveryDay:
                if (m_timePoint.hour != currentHour)
                    return TimeMatchResult::MATCH_FAILED;
                [[fallthrough]];
            case reminder::LoopType::EveryHour:
                if (m_timePoint.minute != currentMinute)
                    return TimeMatchResult::MATCH_FAILED;
                [[fallthrough]];
            default:
                break;
            }
        }
        return TimeMatchResult::MATCH_SUCCEED;
    }

    ScheduleQueue::CheckStatus ScheduleQueue::getNotifications(std::array<std::queue<Schedule>, 2>& schQueues)
    {
        auto tp = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(tp);
        std::tm currentLocalTime = *std::localtime(&currentTime);
        const auto ntflag = getNewTimeFlags(currentLocalTime);
        if (m_firstRun)
            m_firstRun = false;
        else
        {
            if ((ntflag & NewMinute) != NewMinute)
                return CheckStatus::NotCheckYet;
        }
        update(ntflag);
        m_lastUpdateTimePoint = tp;

        bool MatchedOnceEvent = false;
        for (size_t i = 0, sz = m_checkQueue.size(); i < sz; ++i)
        {
            const auto& sch = m_checkQueue[i];
            if (checked(i)) continue;
            const auto matchResult = sch.match(currentLocalTime);
            if (matchResult == TimeMatchResult::MATCH_SUCCEED)
            {
                schQueues[1].push(sch);
                if (sch.m_scheduleType == ScheduleType::Once)
                {
                    m_notifiedCommonQueue.emplace(i);
                    m_historyQueue.emplace_back(sch);
                    MatchedOnceEvent = true;
                }
                else
                {
                    switch (sch.m_loopType)
                    {
                    case LoopType::EveryYear: m_notifiedYearQueue.emplace(i); break;
                    case LoopType::EveryMonth: m_notifiedMonthQueue.emplace(i); break;
                    case LoopType::EveryWeek: m_notifiedWeekQueue.emplace(i); break;
                    case LoopType::EveryDay: m_notifiedDayQueue.emplace(i); break;
                    case LoopType::EveryHour: m_notifiedHourQueue.emplace(i); break;
                    default: break;
                    }
                }
            }
            else if (matchResult == TimeMatchResult::MATCH_OUT_OF_DATE)
            {
                assert(sch.m_scheduleType == ScheduleType::Once);
                schQueues[0].push(sch);
                m_notifiedCommonQueue.emplace(i);
                m_historyQueue.emplace_back(sch);
                MatchedOnceEvent = true;
            }
        }

        return MatchedOnceEvent ? CheckStatus::MatchOnceEvent : CheckStatus::MatchButNoOnceEvent;
    }

    void ScheduleQueue::initQueues()
	{
        std::ifstream file(m_fileName, std::ios_base::in | std::ios_base::binary);
        if (file.is_open())
        {
            // save time
            file.read(reinterpret_cast<char*>(&m_lastUpdateTimePoint), sizeof(m_lastUpdateTimePoint));
            if (file.gcount() < sizeof(m_lastUpdateTimePoint))
            {
                m_lastUpdateTimePoint = std::chrono::system_clock::now();
                file.close();
                return;
            }

            // notify list
            size_t count = 0;
            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            for (size_t i = 0; i < count; ++i)
            {
                Schedule task;
                // m_description
                size_t length = 0;
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_description.resize(length);
                    file.read(task.m_description.data(), length);
                }
                // m_event
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_event.resize(length);
                    file.read(task.m_event.data(), length);
                }
                // m_timePoint
                file.read(reinterpret_cast<char*>(&task.m_timePoint), sizeof(task.m_timePoint));
                // m_mediaURL
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_mediaURL.resize(length);
                    file.read(task.m_mediaURL.data(), length);
                }
                // m_displayEffect
                file.read(reinterpret_cast<char*>(&task.m_displayEffect), sizeof(task.m_displayEffect));
                // m_scheduleType
                file.read(reinterpret_cast<char*>(&task.m_scheduleType), sizeof(task.m_scheduleType));
                // m_loopType
                file.read(reinterpret_cast<char*>(&task.m_loopType), sizeof(task.m_loopType));

                m_checkQueue.emplace_back(task);
            }

            // history
            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            for (size_t i = 0; i < count; ++i)
            {
                Schedule task;
                // m_description
                size_t length = 0;
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_description.resize(length);
                    file.read(task.m_description.data(), length);
                }
                // m_event
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_event.resize(length);
                    file.read(task.m_event.data(), length);
                }
                // m_timePoint
                file.read(reinterpret_cast<char*>(&task.m_timePoint), sizeof(task.m_timePoint));
                // m_mediaURL
                file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
                if (length)
                {
                    task.m_mediaURL.resize(length);
                    file.read(task.m_mediaURL.data(), length);
                }
                // m_displayEffect
                file.read(reinterpret_cast<char*>(&task.m_displayEffect), sizeof(task.m_displayEffect));
                // m_scheduleType
                file.read(reinterpret_cast<char*>(&task.m_scheduleType), sizeof(task.m_scheduleType));
                // m_loopType
                file.read(reinterpret_cast<char*>(&task.m_loopType), sizeof(task.m_loopType));

                m_historyQueue.emplace_back(task);
            }

            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            std::vector<size_t> buffer;
            if (count)
            {
                buffer.resize(count);
                file.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(size_t));
                for (const size_t i : buffer)
                {
                    m_notifiedYearQueue.emplace(i);
                }
            }

            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            if (count)
            {
                buffer.resize(count);
                file.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(size_t));
                for (const size_t i : buffer)
                {
                    m_notifiedMonthQueue.emplace(i);
                }
            }

            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            if (count)
            {
                buffer.resize(count);
                file.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(size_t));
                for (const size_t i : buffer)
                {
                    m_notifiedWeekQueue.emplace(i);
                }
            }

            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            if (count)
            {
                buffer.resize(count);
                file.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(size_t));
                for (const size_t i : buffer)
                {
                    m_notifiedDayQueue.emplace(i);
                }
            }

            file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            if (count)
            {
                buffer.resize(count);
                file.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(size_t));
                for (const size_t i : buffer)
                {
                    m_notifiedHourQueue.emplace(i);
                }
            }
            

            file.close();
        }
        else
        {
            m_lastUpdateTimePoint = std::chrono::system_clock::now();
        }
	}

    void ScheduleQueue::saveQueues()
    {
        std::ofstream file(m_fileName, std::ios_base::out | std::ios_base::binary | std::ios::trunc);
        if (file.is_open())
        {
            // save time
            file.write(reinterpret_cast<const char*>(&m_lastUpdateTimePoint), sizeof(m_lastUpdateTimePoint));
            // save task list
            std::unordered_set<size_t> totalSet;
            for (const size_t i : m_notifiedCommonQueue)
            {
                totalSet.emplace(i);
            }
            for (const size_t i : m_removedCommonQueue)
            {
                totalSet.emplace(i);
            }
            assert(m_checkQueue.size() >= totalSet.size());
            size_t count = m_checkQueue.size() - totalSet.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (size_t i = 0, listSize = m_checkQueue.size(); i < listSize; ++i)
            {
                if (m_notifiedCommonQueue.count(i) != 0 || m_removedCommonQueue.count(i) != 0)
                    continue;

                const auto& task = m_checkQueue[i];
                size_t length = task.m_description.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_description.data(), length);

                length = task.m_event.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_event.data(), length);

                file.write(reinterpret_cast<const char*>(&task.m_timePoint), sizeof(task.m_timePoint));
                
                length = task.m_mediaURL.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_mediaURL.data(), length);

                file.write(reinterpret_cast<const char*>(&task.m_displayEffect), sizeof(task.m_displayEffect));

                file.write(reinterpret_cast<const char*>(&task.m_scheduleType), sizeof(task.m_scheduleType));

                file.write(reinterpret_cast<const char*>(&task.m_loopType), sizeof(task.m_loopType));
            }
            // save history list
            assert(m_historyQueue.size() >= m_removedHistoryQueue.size());
            count = m_historyQueue.size() - m_removedHistoryQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (size_t i = 0, sz = m_historyQueue.size(); i < sz; ++i)
            {
                if (m_removedHistoryQueue.count(i) != 0) continue;

                const auto& task = m_historyQueue[i];
                size_t length = task.m_description.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_description.data(), length);

                length = task.m_event.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_event.data(), length);

                file.write(reinterpret_cast<const char*>(&task.m_timePoint), sizeof(task.m_timePoint));
                
                length = task.m_mediaURL.length();
                file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
                file.write(task.m_mediaURL.data(), length);

                file.write(reinterpret_cast<const char*>(&task.m_displayEffect), sizeof(task.m_displayEffect));

                file.write(reinterpret_cast<const char*>(&task.m_scheduleType), sizeof(task.m_scheduleType));

                file.write(reinterpret_cast<const char*>(&task.m_loopType), sizeof(task.m_loopType));
            }

            count = m_notifiedYearQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (const size_t i : m_notifiedYearQueue)
            {
                file.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
            }

            count = m_notifiedMonthQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (const size_t i : m_notifiedMonthQueue)
            {
                file.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
            }

            count = m_notifiedWeekQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (const size_t i : m_notifiedWeekQueue)
            {
                file.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
            }

            count = m_notifiedDayQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (const size_t i : m_notifiedDayQueue)
            {
                file.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
            }

            count = m_notifiedHourQueue.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
            for (const size_t i : m_notifiedHourQueue)
            {
                file.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
            }

            file.close();
        }
    }
}
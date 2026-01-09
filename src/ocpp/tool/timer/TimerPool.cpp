#include "TimerPool.h"
#include "Timer.h"

#include "utils/utils.h"

namespace ocpp1_6
{
    TimerPool::TimerPool()
        : m_stop(false),
          m_update_wakeup_time(false),
          m_wakeup_mutex(),
          m_wakeup_cond(),
          m_wake_up_time_point(std::chrono::steady_clock::now() + std::chrono::hours(2400u)),
          m_thread(std::bind(&TimerPool::threadLoop, this)),
          m_timers(),
          m_active_timers()
    {
    }

    TimerPool::~TimerPool()
    {
        // 停止线程
        m_stop = true;
        m_wakeup_cond.notify_one();
        m_thread.join();
    }


    Timer *TimerPool::createTimer(const char *name)
    {
        return new Timer(*this, name);
    }

    Timer *TimerPool::getTimer(const std::string &timer_name)
    {
        Timer *timer = nullptr;

        d_log("TimerPool::getTimer: %s", timer_name.c_str());
        std::lock_guard<std::mutex> lock(m_wakeup_mutex);
        for (Timer *t : m_timers)
        {
            if (t->m_name == timer_name)
            {
                timer = t;
                break;
            }
        }
        d_log("TimerPool::getTimer: %s done", timer_name.c_str());
        return timer;
    }

    void TimerPool::threadLoop()
    {
        while (!m_stop)
        {
            std::unique_lock<std::mutex> lock(m_wakeup_mutex);
            if (m_wakeup_cond.wait_until(lock, m_wake_up_time_point, [this]
                                         { return (m_stop || m_update_wakeup_time); }))
            {
                if (m_update_wakeup_time)
                {
                    // 计算下一个唤醒时间点
                    computeNextWakeupTimepoint();
                    m_update_wakeup_time = false;
                }
            }
            else
            {
                // 定时器到期
                Timer *timer = m_active_timers.front();
                if (timer->m_single_shot)
                {
                    // 单次触发：从活跃列表中移除定时器
                    m_active_timers.pop_front();
                    timer->m_started = false;
                }
                else
                {
                    // 周期触发：计算下一次触发时间点
                    timer->m_wake_up_time_point += timer->m_interval;
                }

                // 更新下一个唤醒时间点
                computeNextWakeupTimepoint();

                // 调用回调函数通知用户
                auto cb = timer->m_callback;
                lock.unlock();
                cb();
                lock.lock();
            }
        }
    }

    void TimerPool::computeNextWakeupTimepoint()
    {
        if (m_active_timers.empty())
        {
            // 若无活跃定时器，则设置默认唤醒时间为 100 天后
            m_wake_up_time_point = std::chrono::steady_clock::now() + std::chrono::hours(2400u);
        }
        else
        {
            // 对活跃定时器列表排序，确保最早到期的定时器在前
            m_active_timers.sort([](const Timer *a, const Timer *b)
                                 { return (a->m_wake_up_time_point < b->m_wake_up_time_point); });
            m_wake_up_time_point = m_active_timers.front()->m_wake_up_time_point;
        }
    }


    void TimerPool::registerTimer(Timer *timer)
    {
        std::lock_guard<std::mutex> lock(m_wakeup_mutex);
        m_timers.push_back(timer);
    }

    void TimerPool::lock()
    {
        if (std::this_thread::get_id() != m_thread.get_id())
        {
            m_wakeup_mutex.lock();
        }
    }

    void TimerPool::unlock()
    {
        if (std::this_thread::get_id() != m_thread.get_id())
        {
            m_wakeup_mutex.unlock();
        }
    }


    void TimerPool::addTimer(Timer *timer)
    {
        if (timer->m_wake_up_time_point < m_wake_up_time_point)
        {
            // 若新定时器的唤醒时间早于当前时间点，则更新唤醒时间
            m_update_wakeup_time = true;
            m_wakeup_cond.notify_one();
        }

        m_active_timers.push_back(timer);
    }

    void TimerPool::removeTimer(Timer *timer)
    {
        if (timer == m_active_timers.front())
        {
            // 若移除的是当前即将到期的定时器，则更新唤醒时间
            m_update_wakeup_time = true;
            m_wakeup_cond.notify_one();
        }

        m_active_timers.remove(timer);
    }

    void TimerPool::stopAllTimers()
    {
        std::list<Timer *> activeTimersCopy;

        {
            // 锁定互斥锁并复制活跃定时器列表
            std::lock_guard<std::mutex> lock(m_wakeup_mutex);
            activeTimersCopy = m_active_timers;
        }

        // 遍历副本并停止定时器
        for (Timer *timer : activeTimersCopy)
        {
            timer->stop();
        }

        {
            // 再次锁定互斥锁并清空活跃定时器列表
            std::lock_guard<std::mutex> lock(m_wakeup_mutex);
            m_active_timers.clear();
            computeNextWakeupTimepoint();
            m_update_wakeup_time = true;
            m_wakeup_cond.notify_one();
        }
    }

} // namespace ocpp1_6
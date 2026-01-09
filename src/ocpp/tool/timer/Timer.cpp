#include "Timer.h"
#include "ITimerPool.h"

namespace ocpp1_6
{
    Timer::Timer(ITimerPool &pool, const char *name)
        : m_pool(pool),
          m_name(name),
          m_single_shot(false),
          m_interval(std::chrono::milliseconds(0)),
          m_wake_up_time_point(std::chrono::time_point<std::chrono::steady_clock>::min()),
          m_started(false),
          m_callback()
    {
        m_pool.registerTimer(this);
    }

    Timer::~Timer()
    {
        stop();
    }

    bool Timer::start(std::chrono::milliseconds interval, bool single_shot, bool immediately)
    {
        bool ret = false;

        // 锁定计时器池以确保线程安全。
        m_pool.lock();

        if (!m_started)
        {
            // 配置计时器参数。
            m_interval = interval;
            m_single_shot = single_shot;
            m_wake_up_time_point = immediately ? std::chrono::steady_clock::now() : (std::chrono::steady_clock::now() + m_interval);

            // 将计时器添加到池中。
            m_pool.addTimer(this);

            m_started = true;
            ret = true;
        }

        // 解锁计时器池。
        m_pool.unlock();

        return ret;
    }

    bool Timer::restart(std::chrono::milliseconds interval, bool single_shot, bool immediately)
    {
        bool ret = false;

        // 锁定计时器池以确保线程安全。
        m_pool.lock();

        if (m_started)
        {
            // 从池中移除当前计时器。
            m_pool.removeTimer(this);
        }

        // 配置新的计时器参数。
        m_interval = interval;
        m_single_shot = single_shot;
        m_wake_up_time_point = immediately ? std::chrono::steady_clock::now() : (std::chrono::steady_clock::now() + m_interval);

        // 将计时器重新添加到池中。
        m_pool.addTimer(this);

        m_started = true;
        ret = true;

        // 解锁计时器池。
        m_pool.unlock();

        return ret;
    }

    bool Timer::stop()
    {
        bool ret = false;

        // 锁定计时器池以确保线程安全。
        m_pool.lock();

        if (m_started)
        {
            // 从池中移除计时器。
            m_pool.removeTimer(this);

            m_started = false;
            ret = true;
        }

        // 解锁计时器池。
        m_pool.unlock();

        return ret;
    }

    void Timer::setCallback(std::function<void()> callback)
    {
        // 锁定计时器池以确保线程安全。
        m_pool.lock();

        // 保存回调函数。
        m_callback = callback;

        // 解锁计时器池。
        m_pool.unlock();
    }

} // namespace ocpp1_6
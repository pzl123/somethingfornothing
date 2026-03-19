#ifndef OPENOCPP_TIMERPOOL_H
#define OPENOCPP_TIMERPOOL_H

#include "ITimerPool.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

namespace ocpp1_6
{

  class Timer;

  /**
   * @brief 定时器池类，用于管理一组定时器。
   */
  class TimerPool : public ITimerPool
  {
    friend class Timer;

  public:
    /**
     * @brief 构造函数，初始化定时器池。
     */
    TimerPool();

    /**
     * @brief 析构函数，释放定时器池资源。
     */
    virtual ~TimerPool();

    /**
     * @brief 创建一个新的定时器。
     *
     * @param name 定时器名称（可选）。
     * @return 新创建的定时器对象指针。
     */
    Timer *createTimer(const char *name = "") override;

    /**
     * @brief 根据名称获取定时器。
     *
     * @param timer_name 定时器名称。
     * @return 匹配名称的定时器对象指针，若不存在则返回空。
     */
    Timer *getTimer(const std::string &timer_name) override;

    /**
     * @brief 停止所有定时器。
     */
    void stopAllTimers();

  private:
    /** @brief 标记是否停止所有定时器。 */
    std::atomic<bool> m_stop;

    /** @brief 标记下一个唤醒时间是否已更改。 */
    std::atomic<bool> m_update_wakeup_time;

    /** @brief 唤醒条件的互斥锁。 */
    std::mutex m_wakeup_mutex;

    /** @brief 唤醒条件变量，用于线程同步。 */
    std::condition_variable m_wakeup_cond;

    /** @brief 下一次唤醒的时间点。 */
    std::chrono::time_point<std::chrono::steady_clock> m_wake_up_time_point;

    /** @brief 定时器线程，负责处理定时任务。 */
    std::thread m_thread;

    /** @brief 已注册的定时器列表。 */
    std::list<Timer *> m_timers;

    /** @brief 当前活跃的定时器列表。 */
    std::list<Timer *> m_active_timers;

    /**
     * @brief 定时器线程主循环，处理定时任务。
     */
    void threadLoop();

    /**
     * @brief 计算下一次唤醒的时间点。
     */
    void computeNextWakeupTimepoint();

    /**
     * @brief 注册一个定时器到池中。
     *
     * @param timer 要注册的定时器对象指针。
     */
    void registerTimer(Timer *timer) override;

    /**
     * @brief 锁定定时器池，确保线程安全。
     */
    void lock() override;

    /**
     * @brief 解锁定时器池。
     */
    void unlock() override;

    /**
     * @brief 将定时器添加到活跃列表中。
     *
     * @param timer 要添加的定时器对象指针。
     */
    void addTimer(Timer *timer) override;

    /**
     * @brief 从活跃列表中移除定时器。
     *
     * @param timer 要移除的定时器对象指针。
     */
    void removeTimer(Timer *timer) override;
  };

} // namespace ocpp1_6

#endif // OPENOCPP_TIMERPOOL_H
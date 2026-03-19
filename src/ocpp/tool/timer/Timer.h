#ifndef OPENOCPP_TIMER_H
#define OPENOCPP_TIMER_H

#include <chrono>
#include <functional>
#include <string>

namespace ocpp1_6
{

  class ITimerPool;

  /**
   * @brief 定时器类，用于管理定时任务。
   */
  class Timer
  {
    friend class TimerPool;

  public:
    /**
     * @brief 构造函数，初始化定时器。
     *
     * @param pool 管理定时器的计时器池对象引用。
     * @param name 定时器名称（可选）。
     */
    Timer(ITimerPool &pool, const char *name = "");

    /**
     * @brief 析构函数，释放定时器资源。
     */
    virtual ~Timer();

    /**
     * @brief 启动定时器。
     *
     * @param interval 定时器的时间间隔（毫秒）。
     * @param single_shot 是否为单次触发模式。
     * @param immediately 是否立即启动定时器。
     * @return true 启动成功。
     * @return false 启动失败（例如定时器已启动）。
     */
    bool start(std::chrono::milliseconds interval, bool single_shot = false, bool immediately = true);

    /**
     * @brief 重启定时器。
     *
     * @param interval 定时器的时间间隔（毫秒）。
     * @param single_shot 是否为单次触发模式。
     * @param immediately 是否立即启动定时器。
     * @return true 重启成功。
     * @return false 重启失败。
     */
    bool restart(std::chrono::milliseconds interval, bool single_shot = false, bool immediately = true);

    /**
     * @brief 停止定时器。
     *
     * @return true 停止成功。
     * @return false 停止失败（例如定时器未启动）。
     */
    bool stop();

    /**
     * @brief 检查定时器是否已启动。
     *
     * @return true 定时器已启动。
     * @return false 定时器未启动。
     */
    bool isStarted() const { return m_started; }

    /**
     * @brief 检查定时器是否为单次触发模式。
     *
     * @return true 定时器为单次触发模式。
     * @return false 定时器为周期触发模式。
     */
    bool isSingleShot() const { return m_single_shot; }

    /**
     * @brief 设置定时器的回调函数。
     *
     * @param callback 定时器到期时调用的函数。
     */
    void setCallback(std::function<void()> callback);

    /**
     * @brief 获取定时器的回调函数。
     *
     * @return 定时器到期时调用的函数。
     */
    const std::function<void()> &getCallback() const { return m_callback; }

    /**
     * @brief 获取定时器的时间间隔。
     *
     * @return 定时器的时间间隔。
     */
    const std::chrono::milliseconds &getInterval() const { return m_interval; }

    /**
     * @brief 获取定时器的名称。
     *
     * @return 定时器的名称。
     */
    const std::string &getName() const { return m_name; }

  private:
    /** @brief 计时器池对象引用，用于管理计时器。 */
    ITimerPool &m_pool;

    /** @brief 定时器名称，用于标识计时器。 */
    const std::string m_name;

    /** @brief 标记计时器是否为单次触发模式。 */
    bool m_single_shot;

    /** @brief 定时器的时间间隔（毫秒）。 */
    std::chrono::milliseconds m_interval;

    /** @brief 下一次触发的时间点。 */
    std::chrono::time_point<std::chrono::steady_clock> m_wake_up_time_point;

    /** @brief 标记计时器是否已启动。 */
    bool m_started;

    /** @brief 定时器到期时调用的回调函数。 */
    std::function<void()> m_callback;
  };

} // namespace ocpp1_6

#endif // OPENOCPP_TIMER_H
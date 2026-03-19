#ifndef OPENOCPP_ITIMERPOOL_H
#define OPENOCPP_ITIMERPOOL_H

#include <string>

namespace ocpp1_6
{

class Timer;

/**
 * @brief 定时器池接口类，用于实现定时器池功能
 */
class ITimerPool
{
    friend class Timer;

  public:
    /**
     * @brief 析构函数
     */
    virtual ~ITimerPool() { }

    /**
     * @brief 创建一个定时器
     * 
     * @param name 定时器名称（可选）
     * @return 成功返回创建的定时器指针，失败返回 nullptr
     */
    virtual Timer* createTimer(const char* name = "") = 0;

    /**
     * @brief 根据名称获取定时器
     * 
     * @param timer_name 定时器名称
     * @return 找到返回对应的定时器指针，未找到返回 nullptr
     */
    virtual Timer* getTimer(const std::string& timer_name) = 0;

    /**
     * @brief 停止所有定时器。
     */
    virtual void stopAllTimers() = 0;

  protected:
    /**
     * @brief 在定时器池中注册一个定时器
     * 
     * @param timer 需要注册的定时器
     */
    virtual void registerTimer(Timer* timer) = 0;

    /**
     * @brief 锁定对定时器的访问
     */
    virtual void lock() = 0;

    /**
     * @brief 解锁对定时器的访问
     */
    virtual void unlock() = 0;

    /**
     * @brief 将定时器添加到活动定时器列表中
     * 
     * @param timer 需要添加的定时器
     */
    virtual void addTimer(Timer* timer) = 0;

    /**
     * @brief 从活动定时器列表中移除定时器
     * 
     * @param timer 需要移除的定时器
     */
    virtual void removeTimer(Timer* timer) = 0;
};

} // namespace ocpp1_6

#endif // OPENOCPP_ITIMERPOOL_H
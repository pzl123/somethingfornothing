#include <string>
#include <functional>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <future>
#include <memory>
#include <condition_variable>
#include <chrono>
#include "utils/utils.h"

class ThreadPool
{
public:
    ThreadPool(size_t coreThreads = 4,
               size_t maxThreads = 8,
               std::chrono::seconds idleTimeout = std::chrono::seconds(60))
               :m_coreThreads(coreThreads),
               m_maxThreads(maxThreads),
               m_idleTimeout(idleTimeout)
    {
        if (m_coreThreads == 0 || m_maxThreads < m_coreThreads)
        {
            d_log("error num thread");
        }

        for (int i = 0; i < m_coreThreads; i++)
        {
            createWorkThread();
        }
    }

    ~ThreadPool()
    {
        forceShutdown();
    }

    void forceShutdown()
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_isStopping = true;
        m_isForceStopping = true;

        while (!m_taskQueue.empty())
        {
            m_taskQueue.pop();
        }
        m_queuecond.notify_all();
        cleanupFinishedThreads();
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_isStopping = true;
        }
        m_queuecond.notify_all();

        cleanupFinishedThreads();
    }

    template<typename F, typename... Args>
    auto addTask(const std::string& taskId, F&& f, Args&&... arg) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [f = std::forward<F>(f), arg...]() mutable -> return_type
            {
                return f(arg...);
            });
        std::future<return_type> future = task->get_future();

        auto cancelledFlag = std::make_shared<std::atomic<bool>>(false);

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_isStopping)
            {
                throw std::runtime_error("ThreadPool is stop");
            }

            if (!taskId.empty())
            {
                auto it = m_taskMap.find(taskId);
                if (m_taskMap.end() != it)
                {
                    it->second.m_cancelled->store(true);
                }
            }

            TaskWrapper taskWrapper;
            taskWrapper.m_id = taskId;
            taskWrapper.m_cancelled = cancelledFlag;
            taskWrapper.m_func = [task, cancelledFlag]()
            {
                if (true == cancelledFlag->load())
                {
                    return;
                }
                else
                {
                    (*task)();
                }
            };

            m_taskQueue.push(taskWrapper);
        }
        m_queuecond.notify_one();
        adjustWorks();
        return future;
    }

    bool cancelTask(const std::string& taskId)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(m_queueMutex);
        auto it = m_taskMap.find(taskId);
        if ((m_taskMap.end() != it) && (false == it->second.m_cancelled->load()))
        {
            it->second.m_cancelled->store(true);
            ret = true;
        }
        return ret;
    }


private:
    struct TaskWrapper
    {
        std::string m_id;
        std::shared_ptr<std::atomic<bool>> m_cancelled;
        std::function<void()> m_func;
    };

    std::unordered_map<std::thread::id, std::thread> m_workers; /* 工作线程 */
    std::queue<std::thread::id> m_finishedThreadId; /* 已结束的线程 */
    std::mutex m_finishedMutex; /* 包含 m_finishedThreadId */

    std::queue<TaskWrapper> m_taskQueue; /* 任务队列 */
    std::unordered_map<std::string, TaskWrapper> m_taskMap;

    std::mutex m_queueMutex; /* 任务队列锁 */
    std::condition_variable m_queuecond; /* 任务队列条件变量 */

    size_t m_coreThreads; /* 线程数量 */
    size_t m_maxThreads; /* 线程最大数量 */
    size_t m_currentThreads; /* 当前线程工作总数量 */
    size_t m_initialQueueSize; /* 队列初始化容量 */
    size_t m_currentQueueCapacity; /* 队列当前容量 */
    size_t m_maxQueueSize; /* 队列最大容量 */
    size_t m_queueResizeFactor; /* 队列 */

    std::chrono::seconds m_idleTimeout; /* 空闲时间 */

    std::atomic<bool> m_isStopping;
    std::atomic<bool> m_isForceStopping;
    std::atomic<size_t> m_activeThreads{0};

    void workloop()
    {
        std::thread::id self_id = std::this_thread::get_id();
        while (true)
        {
            TaskWrapper task;
            bool has_task;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                bool predicate_satisfied = m_queuecond.wait_for(lock, m_idleTimeout, [this]()
                                                               {
                                                                   return m_isStopping || m_isForceStopping || !m_taskQueue.empty();
                                                               });
                if (false == predicate_satisfied) /* 超时 */
                {
                    if ((m_currentThreads > m_coreThreads) || m_isForceStopping)
                    {
                        m_currentThreads--;
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(self_id); /* 标识自己退出 */
                            d_log("Thread %zu timeout, marked for cleanup", std::hash<std::thread::id>{}(self_id));
                        }
                        return; /* 当前线程超时并且当前线程数大于核心线程数时直接退出该线程, 线程池缩容 */
                    }
                    else
                    {
                        continue;
                    }
                }
                else /* 当谓词为true 停止信号或者队列非空有任务 */
                {
                    if (m_isForceStopping)
                    {
                        /* 强制停止, 直接退出 */
                        m_currentThreads--;
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(std::this_thread::get_id()); /* 标识自己退出 */
                            d_log("Thread %zu force stop, marked for cleanup", std::hash<std::thread::id>{}(self_id));
                        }
                        return;
                    }

                    if (m_isStopping && m_taskQueue.empty()) /* 停止并且队列为空 */
                    {
                        m_currentThreads--;
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(std::this_thread::get_id()); /* 标识自己退出 */
                            d_log("Thread %zu m_isStopping && m_taskQueue empty, marked for cleanup", std::hash<std::thread::id>{}(self_id));

                        }
                        return;
                    }

                    if (!m_taskQueue.empty())
                    {
                        /* 任务队列非空 取任务 处理 */
                        task = m_taskQueue.front();
                        m_taskQueue.pop();
                        has_task = true;
                    }
                }
            }

            if (has_task) /* 处理任务 */
            {
                if ((nullptr != task.m_cancelled) && (true == task.m_cancelled->load())) /* 任务取消 */
                {
                    continue;
                }

                m_activeThreads++;
                try
                {
                    task.m_func();
                }
                catch(const std::exception& e)
                {
                    d_log("some error %s", e.what());
                }
                catch(...)
                {
                    /*  防止任务异常影响线程池 */
                }
                m_activeThreads--;
            }
        }
    }

    void createWorkThread()
    {
        std::thread t(&ThreadPool::workloop, this);
        std::thread::id tid = t.get_id();
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_workers.emplace(tid, std::move(t));
            m_currentThreads++;
        }
    }

    void cleanupFinishedThreads()
    {
        std::queue<std::thread::id> finishedCopy;
        {
            std::lock_guard<std::mutex> lock(m_finishedMutex);
            finishedCopy.swap(m_finishedThreadId);
        }

        while (!finishedCopy.empty())
        {
            std::thread::id id = finishedCopy.front();
            finishedCopy.pop();
            auto it = m_workers.find(id);
            if (it != m_workers.end())
            {
                if (it->second.joinable())
                {
                    it->second.join();
                }
                d_log("thread %zu erase in m_workers", std::hash<std::thread::id>{}(id));
                m_workers.erase(it);
            }
        }
    }

    void adjustWorks()
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_taskQueue.size() > m_currentThreads && m_currentThreads < m_maxThreads) /* 当前任务数大于线程数并且线程数量小于最大值 */
        {
            size_t target = std::min(m_maxThreads, m_currentThreads * 2);
            for (size_t i = m_currentThreads; i < target; i++)
            {
                createWorkThread();
            }
        }
        cleanupFinishedThreads();
    }

};

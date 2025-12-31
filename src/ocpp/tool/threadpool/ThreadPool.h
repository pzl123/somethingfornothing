#ifndef THREADPOOL_H
#define THREADPOOL_H

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
#include <cstdint>
#include <pthread.h>
#include "utils/utils.h"

class ThreadPool
{
public:
    ThreadPool(size_t coreThreads = 4,
               size_t maxThreads = 8,
               std::chrono::seconds idleTimeout = std::chrono::seconds(60))
               :m_coreThreads(coreThreads),
               m_maxThreads(maxThreads),
               m_currentThreads(0),
               m_idleTimeout(idleTimeout),
               m_isStopping(false),
               m_isForceStopping(false),
               m_activeThreads(0)
    {
        if (m_coreThreads == 0 || m_maxThreads < m_coreThreads)
        {
            d_log("error num thread");
        }

        for (uint i = 0; i < m_coreThreads; i++)
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
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_isStopping = true;
            m_isForceStopping = true;

            while (!m_taskQueue.empty())
            {
                m_taskQueue.pop();
            }
            m_queuecond.notify_all();
        }
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        // cleanupFinishedThreads();
        std::vector<std::thread> threadsToJoin;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            for (auto &kv : m_workers)
            {
                if (kv.second.joinable())
                {
                    threadsToJoin.push_back(std::move(kv.second));
                }
            }
            m_workers.clear();
        }

        for (auto &t : threadsToJoin)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        d_log("ThreadPool forceShutdown completed");
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
        adjustWorks();
        m_queuecond.notify_one();
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

    size_t getCurrentThreads()
    {
        return m_currentThreads;
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
    std::atomic<size_t> m_currentThreads{0};; /* 当前线程工作总数量 */

    std::chrono::seconds m_idleTimeout; /* 空闲时间 */

    std::atomic<bool> m_isStopping;
    std::atomic<bool> m_isForceStopping;
    std::atomic<size_t> m_activeThreads{0};

    void workloop()
    {
        std::thread::id self_id = std::this_thread::get_id();
        // d_log("Worker thread %zu started", std::hash<std::thread::id>{}(self_id));
        while (true)
        {
            TaskWrapper task;
            bool has_task = false;
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
                        m_currentThreads.fetch_sub(1, std::memory_order_relaxed);
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(self_id); /* 标识自己退出 */
                            // d_log("Thread %zu timeout, marked for cleanup", std::hash<std::thread::id>{}(self_id));
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
                        m_currentThreads.fetch_sub(1, std::memory_order_relaxed);
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(std::this_thread::get_id()); /* 标识自己退出 */
                            // d_log("Thread %zu force stop, marked for cleanup", std::hash<std::thread::id>{}(self_id));
                        }
                        return;
                    }

                    if (m_isStopping && m_taskQueue.empty()) /* 停止并且队列为空 */
                    {
                        m_currentThreads.fetch_sub(1, std::memory_order_relaxed);
                        {
                            std::lock_guard<std::mutex> finishLock(m_finishedMutex);
                            m_finishedThreadId.push(std::this_thread::get_id()); /* 标识自己退出 */
                            // d_log("Thread %zu m_isStopping && m_taskQueue empty, marked for cleanup", std::hash<std::thread::id>{}(self_id));

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
        static std::atomic<int> s_workerCounter{0};
        int id = ++s_workerCounter;
        std::thread t([this, id]()
                      {
        // 设置线程名（最多 15 个字符 + '\0'）
        char threadName[16];
        snprintf(threadName, sizeof(threadName), "Worker-%d", id);
        pthread_setname_np(pthread_self(), threadName);

        this->workloop(); });
        // d_log("Creating new worker thread...");
        std::thread::id tid = t.get_id();
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_workers.emplace(tid, std::move(t));
        }
        m_currentThreads.fetch_add(1, std::memory_order_relaxed);
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
        size_t taskQueueSize = 0;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            taskQueueSize = m_taskQueue.size();
        }
        if (taskQueueSize > m_currentThreads && m_currentThreads < m_maxThreads) /* 当前任务数大于线程数并且线程数量小于最大值 */
        {
            size_t target = std::min(m_maxThreads, m_currentThreads * 2);
            for (size_t i = m_currentThreads; i < target; i++)
            {
                createWorkThread();
            }
        }
        else
        {
            // d_log("No scaling: queue=%u, current=%u, max=%u", taskQueueSize, m_currentThreads.load(), m_maxThreads);
        }
        cleanupFinishedThreads();
    }

};

#endif /* THREADPOOL_H */

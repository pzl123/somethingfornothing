#ifndef DBEXECUTOR_H
#define DBEXECUTOR_H
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class DBExecutor
{
  public:
    static DBExecutor& instance()
    {
        static DBExecutor inst;
        return inst;
    }

    template <typename F>
    auto postAndWait(F&& task) -> decltype(task())
    {
        using R = decltype(task());

        // 使用 shared_ptr 管理 task，保证捕获 const lambda 安全
        auto ptask  = std::make_shared<std::packaged_task<R()>>(std::forward<F>(task));
        auto future = ptask->get_future();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.emplace([ptask]() { (*ptask)(); });
        }

        m_cv.notify_one();
        return future.get();
    }

    bool isRunning() const { return m_running; }

  private:
    DBExecutor()
    {
        m_worker = std::thread([this]() { run(); });
    }

    ~DBExecutor()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }
        m_cv.notify_all();
        if (m_worker.joinable())
            m_worker.join();
    }

    DBExecutor(const DBExecutor&)            = delete;
    DBExecutor& operator=(const DBExecutor&) = delete;

    void run()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [&] { return !m_tasks.empty() || !m_running; });

                if (!m_running && m_tasks.empty())
                    return;

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
            task();
        }
    }

  private:
    std::thread                       m_worker;
    std::mutex                        m_mutex;
    std::condition_variable           m_cv;
    std::queue<std::function<void()>> m_tasks;
    bool                              m_running = true;
};

#endif // DBEXECUTOR_H

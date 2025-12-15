#include "ThreadPool.h"

#include <thread>
#include <iostream>
#include <unistd.h>
#include <cassert>


void push_task()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Task executed!" << std::endl;
}

int slow_task(int id, int ms = 100)
{
    d_log("Task %d started", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    d_log("Task %d finished", id);
    return id * 10;
}

// 抛出异常的任务
void throwing_task()
{
    throw std::runtime_error("Oops! Task failed!");
}

int main(void)
{
    // d_log("=== Test 0: Basic task execution ===");
    // {
    //     ThreadPool pool(2, 4);
    //     auto f = pool.addTask("task1", []() {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //         return 42;
    //     });

    //     int result = f.get(); // 阻塞等待并获取结果
    //     assert(result == 42);
    //     d_log("Basic task returned: %d", result);
    // }

    // d_log("=== Test 1: Basic task execution ===");
    // {
    //     ThreadPool pool(2, 4);
    //     auto f1 = pool.addTask("t1", slow_task, 1, 50);
    //     auto f2 = pool.addTask("t2", slow_task, 2, 50);

    //     int r1 = f1.get();
    //     int r2 = f2.get();

    //     assert(r1 == 10);
    //     assert(r2 == 20);
    //     d_log("Basic tasks passed: %d, %d", r1, r2);
    // }

    // d_log("=== Test 2: Task cancellation ===");
    // {
    //     ThreadPool pool(1, 2);
    //     auto f1 = pool.addTask("long_task", []()
    //                            {
    //         std::this_thread::sleep_for(std::chrono::seconds(5));
    //         return 999; });

    //     bool cancelled = pool.cancelTask("long_task");
    //     d_log("Cancelled long_task %s", cancelled ? "yes" : "no");

    //     auto f2 = pool.addTask("long_task", []()
    //                            {
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //         return 888; });

    //     int result = f2.get();
    //     assert(result == 888);
    //     d_log("Cancellation test passed, got: %d", result);
    // }

    // d_log("=== Test 3: Exception safety ===");
    // {
    //     ThreadPool pool(2, 4);
    //     auto f = pool.addTask("bad_task", throwing_task);
    //     try
    //     {
    //         f.get(); // packaged_task 会 rethrow exception
    //     }
    //     catch (const std::exception &e)
    //     {
    //         d_log("Caught expected exception: %s", e.what());
    //     }
    //     // 线程池应仍可用
    //     auto f2 = pool.addTask("", []()
    //                            { return 42; });
    //     assert(f2.get() == 42);
    //     d_log("Exception safety test passed");
    // }

    // d_log("=== Test 4: Dynamic scaling (beyond core threads) ===");
    // {
    //     ThreadPool pool(1, 4); // 1 core, max 4
    //     std::vector<std::future<int>> futures;
    //     const int N = 6;
    //     for (int i = 0; i < N; ++i)
    //     {
    //         std::string taskid = std::to_string(i);
    //         futures.push_back(pool.addTask(taskid, slow_task, i, 200));
    //         // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //         d_log("current threads: %u", pool.getCurrentThreads());
    //     }

    //     int sum = 0;
    //     for (auto &f : futures)
    //     {
    //         sum += f.get();
    //     }
    //     d_log("Scaled to handle %d tasks, sum = %d", N, sum);
    //     assert(sum == 150);
    // }

    d_log("=== Test 5: Idle timeout shrink (non-core threads die) ===");
    {
        ThreadPool pool(1, 4, std::chrono::seconds(2)); // idle=2s
        d_log("Initial threads: %zu", pool.getCurrentThreads());

        for (int i = 0; i < 4; ++i)
        {
            pool.addTask("", []() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 等待任务完成
        d_log("After burst: threads = %zu", pool.getCurrentThreads());

        // 等待 >2s，非核心线程应退出
        std::this_thread::sleep_for(std::chrono::seconds(3));
        d_log("After idle timeout: threads = %zu", pool.getCurrentThreads());

        // 应回到 coreThreads = 1
        assert(pool.getCurrentThreads() == 1);
        d_log("Idle shrink test passed");
    }
    return 0;
}
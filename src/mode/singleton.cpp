#include "singleton.h"

#include <iostream>
#include <thread>
#include <mutex>
// 初始化静态成员变量
namespace lazySingleton
{
    Singleton *Singleton::instance = nullptr;
    Singleton::Singleton(int val):value(val){};
    Singleton* Singleton::getInstance()
    {
        if (instance == nullptr)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);
            if (instance == nullptr)
            {
                instance = new Singleton(1);
            }
        }
        return instance;
    };
    /* static Singleton& getInstance() {
        static Singleton instance(1); // 静态局部变量，线程安全的初始化
        return instance;
    }; */

    void Singleton::printValue() const
    {
        std::cout << "LazySingleton  value: " << value << std::endl;
    };
}

namespace preSingleton
{
    // 定义并初始化静态成员变量
    Singleton Singleton::instance(1); // 初始化值为1

    Singleton::Singleton(int val) : value(val) {}

    Singleton& Singleton::getInstance() {
        return instance;
    }

    void Singleton::printValue() {
        std::cout << "PreSingleton Value: " << value << std::endl;
    }
}

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

namespace lazySingleton
{
    class Singleton
    {
    private:
        static Singleton* instance;
        int value;
        Singleton(int val);
    public:
        // 删除拷贝构造函数和赋值操作符，防止复制实例
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        static Singleton* getInstance();
        void printValue() const;
    };
}

namespace preSingleton
{
    class Singleton
    {
    private:
        static Singleton instance; // 静态实例，在程序启动时初始化
        int value;
        // 私有构造函数，防止外部实例化
        Singleton(int val);
    public:
        Singleton(const Singleton&) = delete;
        Singleton & operator=(const Singleton&) = delete;
        static Singleton& getInstance();
        void printValue();
    };
}


#endif /* __SINGLETON_H__ */

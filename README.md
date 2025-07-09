# somethingfornothing


# Cmake create project

创建构建目录

```shell
    mkdir build && cd build
    # 指定 x86_64-linux-gnu 平台
    cmake -DCMAKE_TOOLCHAIN_FILE=../platform/x86_64.cmake ..
    # 开启 asan
    cmake -DCMAKE_TOOLCHAIN_FILE=../platform/x86_64.cmake -DBUILD_ASAN=ON ..
    # LOG_STDOUT 输出到TTL, -LOG_LEVEL=0-3 :DEBUG INFO WARN ERROR DEFAULT
    cmake -DCMAKE_TOOLCHAIN_FILE=../platform/x86_64.cmake -DLOG_STDOUT=ON -LOG_LEVEL=4 ..
```

# Start lighttpd
```shell
    sudo ./lighttpd -f ../config/lighttpd.conf -m ../lib

    ps aux | grep lighttpd | grep -v grep | awk '{print $2}' | xargs sudo kill -9
```

# utils 
## timer.c用法
```
1.在main开始时 p_timer_init();初始化定时器
2.在需要位置 Timer_handle_t t1 = p_timer_add()创建一个定时器,即添加了一个定时任务
3.创建一个定时任务, 默认已使用p_timer_start开始, 只创建不想使用, 创建完后使用p_timer_stop停止
    即Timer_handle_t t1 = p_timer_add();
      p_timer_stop(t1);
4.删除一个定时任务p_timer_del();

static void my_callback(void *userdata)
{
    d_log("Timer1 fired! Data: %s", (char*)userdata);
}

static void my_callback1(void *userdata)
{
    d_log("Timer2 fired! Data: %s", (char*)userdata);
}


static void test_timer(void)
{
    int quit = 0;
    p_timer_init(&quit);
    char *data = strdup("Hello Timer");
    Timer_handle_t t1 = p_timer_add(1000, 3, &my_callback, data);
    Timer_handle_t t2 = p_timer_add(1000, 3, &my_callback1, data);
    // // 主线程做其他工作
    while (1)
    {
        sleep(5);
    }
    quit = 1;
    p_timer_del(&t1);
    p_timer_del(&t2);
}

int main(void)
{
    test_timer();
    return 0;
}

```
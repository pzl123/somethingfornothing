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

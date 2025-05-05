# somethingfornothing


# Cmake create project

创建构建目录

```shell
   mkdir build && cd build
    # 指定 x86_64-linux-gnu 平台
    cmake -DCMAKE_TOOLCHAIN_FILE=../platform/x86_64.cmake ..
    # 开启 asan
    cmake -DCMAKE_TOOLCHAIN_FILE=../platform/x86_64.cmake -DBUILD_ASAN=ON ..
```

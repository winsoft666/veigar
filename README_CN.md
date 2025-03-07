# Veigar

Veigar 是一个跨平台的远程过程调用（RPC）框架，目前支持 Windows、Linux 平台。

Veigar 基于共享内存技术实现，只支持本机进程或线程间的远程过程调用，这是 Veigar 与其他 RPC 框架（如 Thrift、grpc）的最大不同之处。

> 据我所知，Veigar 是第一个开源的基于共享内存技术的RPC框架。
> 
> Veigar 一词来源于英雄联盟里面的“邪恶小法师-维迦”。

Veigar 具有如下特性：

- 可以将任何函数暴露给调用方（不限语言，只要实现 msgpack-rpc 即可）。

- 任何语言编写的程序都可以调用被暴露的函数。

- 不需要学习 IDL 语言。

- 不需要添加额外的代码生成步骤，仅需要 C++ 代码。

- 没有服务端和客户端的概念，每个 Veigar 实例间都可以相互调用。

- 不需要考虑网络、端口可用性等问题，特别是 Windows 系统上的端口假可用性问题。

- 提供了三种调用方式：同步、异步Promise、异步Callback。

# 编译
虽然 Veigar 的底层是基于`msgpack`实现的，但已经将其包含到项目中，不需要额外编译和安装`msgpack`。

虽然在 Veigar 公共头文件引用了 msgpack 头文件，但这不会污染您的全局msgpack命名空间，因为 Veigar 中的 msgpack 命令空间为`veigar_msgpack`。

可以使用 [CMake](https://cmake.org/) 进行编译构建，也可以使用 [vcpkg](https://github.com/microsoft/vcpkg) 进行安装，如：

```bash
vcpkg install veigar
```

# 快速上手

在使用Veigar时，仅需要在项目中包含`include`目录，并链接静态库即可。

## 同步调用

下面是一个同步调用的示例：

> 本示例为了使代码更加简洁，没有对函数返回值进行校验，在实际项目中不要这样做！

```cpp
#include <iostream>
#include "veigar/veigar.h"

using namespace veigar;

int main(int argc, char** argv) {
    if (argc != 3) {
        return 1;
    }

    std::string channelName = argv[1];
    std::string targetChannelName = argv[2];

    Veigar vg;

    vg.bind("echo", [](const std::string& msg, int i, double d, std::vector<uint8_t> buf) {
        std::string result;
        // ...
        return result;
    });

    vg.init(channelName);

    std::vector<uint8_t> buf;
    CallResult ret = vg.syncCall(targetChannelName, 100, "echo", "hello", 12, 3.14, buf);
    if (ret.isSuccess()) {
        std::cout << ret.obj.get().as<std::string>() << std::endl;
    }
    else {
        std::cout << ret.errorMessage << std::endl;
    }

    vg.uninit();

    return 0;
}
```

每个 Veigar 实例有一个在本机范围内唯一的通道名称（Channel），在调用 `init` 函数时需要为 Veigar 指定通道名称，Veigar 不会检测通道的唯一性，需要由调用者来保证通道名称的唯一性。

在上述示例中，需要通过命令行参数指定当前实例的通道名称和目标实例的通道名称，如：

```bash
sample.exe myself other
```

每个实例都绑定了名为`echo`的函数，该函数简单的原样返回msg参数字符串。

通过为`syncCall`函数指定“目标通道名称”、“函数名称”、“函数参数”及“超时毫秒数”就可以同步调用目标函数并得到调用结果。

## 返回Promise的异步调用

使用`asyncCall`函数可以实现异步调用。

下面是返回 Promise 的异步调用示例：

```cpp
std::vector<uint8_t> buf;
std::shared_ptr<AsyncCallResult> acr = vg.asyncCall(targetChannelName, "echo", "hello", 12, 3.14, buf);
if (acr->second.valid()) {
    auto waitResult = acr->second.wait_for(std::chrono::milliseconds(100));
    if (waitResult == std::future_status::timeout) {
        // timeout
    }
    else {
        CallResult ret = std::move(acr->second.get());
        if(ret.isSuccess()) {
            std::cout << ret.obj.get().as<std::string>() << std::endl;
        }
        else {
            std::cout << ret.errorMessage << std::endl;
        }
    }
}

vg.releaseCall(acr->first);
```

与同步调用不同，`asyncCall`函数返回的是`std::shared_ptr<AsyncCallResult>`，而且调用者在获取到`CallResult`或不再关系调用结果时，需要调用`releaseCall`函数释放资源。

## 基于回调函数的异步调用

使用`asyncCall`函数同样可以实现基于回调函数的异步调用。

下面是基于回调函数的异步调用示例：

```cpp
std::vector<uint8_t> buf;
vg.asyncCall([](const CallResult& cr) {
    if(cr.isSuccess()) {
        std::cout << cr.obj.get().as<std::string>() << std::endl;
    }
    else {
        std::cout << cr.errorMessage << std::endl;
    }
 }, targetChannelName, "echo", "hello", 12, 3.14, buf);

```

该方式不需要调用`releaseCall`函数释放资源。

## RPC函数参数类型

支持常规的 C++ 数据类型，如：

- bool
- char, wchar_t
- int, unsigned int, long, unsigned long, long long, unsigned long long
- uint8_t, int8_t, int32_t, uint32_t, int64_t, uint64_t
- float, double

```cpp
 Veigar vg;
 vg.bind("func", [](char c, wchar_t w, int i, int8_t j, int64_t k) {
     // ......
 });
```

也支持如下 STL 数据类型：
- std::string
- std::set
- std::vector
- std::map
- std::string_view (C++ 17)
- std::optional (C++ 17)
- 不支持 std::wstring，但是我们可以使用 std::vector<uint8_t> 来代替 std::wstring

```cpp
 Veigar vg;
 vg.bind("func", [](std::string s, std::vector<std::string>, std::string_view v, std::map<int, bool> m) {
     // ......
 });
```

也可以支持自定义数据类型，如：
```cpp
#include "veigar/msgpack/adaptor/define.hpp"

struct MyPoint {
    int x;
    int y;
    MSGPACK_DEFINE(x, y);
};

Veigar vg;
vg.bind("func", [](MyPoint m) {
    // ......
});
```

详细的参数绑定方法见 [tests/t_type.cpp](./tests/t_type.cpp)。

# 拒绝异常

我不喜欢异常，Veigar 不会通过抛出异常的形式来返回错误，相反 Veigar 会主动捕获所有 C++ 标准库、msgpack 的异常，并以返回值的形式返回给调用者。

当调用失败时（`!ret.isSuccess()`)，errorMessage 中存储的错误信息就可能是 Veigar 捕获的异常信息。

# 性能

使用 `examples\performance-test` 程序作为测试用例：

进程 A 使用 4 个线程同时进行同步调用进程 B，每个线程调用 25000 次，平均每次“调用 <--> 返回结果”消耗 12 微妙。

```txt
Used: 1s240ms721μs, Total: 100000 Success: 100000, Timeout: 0, Failed: 0, Average: 12μs/call.
```

虽然 Veigar 在性能上还有一定的优化提升空间，但就测试结果来看，目前已经远远超越了其他 RPC 框架。

# 赞助名单

感谢您能使用本项目，如果这个项目能对您产生帮助，对我而言也是一件非常开心的事情。

特别感谢下面的赞助者：

- sxzxs (https://github.com/sxzxs)

**可以前往我的 Github [主页](https://github.com/winsoft666) 进行赞助。**
# 1. Veigar
Veigar一词来源于英雄联盟里面的“邪恶小法师-维迦”。

![Veigar on LOL](./veigar-lol.jpg)

Veigar是一个跨平台的远程过程调用（RPC）框架，目前支持Windows、Linux平台。

Veigar基于共享内存技术实现，只支持本机进程或线程间的远程过程调用，这是Veigar与其他RPC框架（如Thrift、grpc）的最大不同之处。

> 据我所知，Veigar是第一个开源的基于共享内存技术的RPC框架。

# 2. 优势

与其他RPC框架相比，Veigar的优势在于：

- 可以将任何函数暴露给调用方（不限语言，只要实现msgpack-rpc即可）。

- 任何语言编写的程序都可以调用被暴露的函数。

- 不需要学习IDL语言。

- 不需要添加额外的代码生成步骤，仅需要C++代码。

- 没有服务端和客户端的概念，每个Veigar实例间都可以相互调用。

- 没有网络问题，如端口占用、半关闭状态等。

- 没有诡异的端口假可用性问题（特别是在Windows系统上）。

# 3. 编译
虽然Veigar的底层是基于`msgpack`实现的，但已经将其包含到项目中，不需要额外编译和安装`msgpack`。

Veigar仅支持编译为静态库。

可以使用CMake进行编译构建，也可以使用[vcpkg](https://github.com/microsoft/vcpkg)进行安装，如：
```bash
vcpkg install veigar
```

# 4. 快速上手

在使用Veigar时，仅需要在项目中包含`include`目录，并链接静态库即可。

## 4.1 同步调用

下面是一个同步调用的示例：

> 本示例为了使代码更加简洁，没有对函数返回值进行校验，请在实际使用中不要这样做！

```cpp
#include <iostream>
#include "veigar/veigar.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        return 1;
    }

    std::string channelName = argv[1];
    std::string targetChannelName = argv[2];

    veigar::Veigar vg;

    vg.bind("echo", [](const std::string& msg, int i, double d, std::vector<uint8_t> buf) {
        std::string result;
        // ...
        return result;
    });

    vg.init(channelName);

    std::vector<uint8_t> buf;
    veigar::CallResult ret = vg.syncCall(targetChannelName, 100, "echo", "hello", 12, 3.14, buf);
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

每个Veigar实例有一个在本机范围内唯一的通道名称（Channel），在调用`init`函数时需要为Veigar指定通道名称，Veigar不会检测通道的唯一性，需要由调用者来保证通道名称的唯一性。

在上述示例中，需要通过命令行参数指定当前实例的通道名称和目标实例的通道名称，如：

```bash
sample.exe myself other
```

每个实例都绑定了名为`echo`的函数，该函数简单的原样返回msg参数字符串。

通过为`syncCall`函数指定“目标通道名称”、“函数名称”、“函数参数”及“超时毫秒数”就可以同步调用目标函数并得到调用结果。

## 4.2 拒绝异常

我不喜欢异常，因此Veigar也不会通过异常的形式来抛出错误，Veigar会主动捕获所有C++标准库、msgpack、boost异常，以返回值的形式返回给调用者。当调用失败时（`!ret.isSuccess()`)，errorMessage中存储的错误信息就可能是Veigar捕获的异常信息。

## 4.3 异步调用
使用`asyncCall`函数可以实现异步调用。

下面是异步调用示例：
```cpp
//
// 与同步调用相同
// ...

std::shared_ptr<veigar::AsyncCallResult> acr = vg.asyncCall(targetChannelName, "echo", msg);
if (acr->second.valid()) {
    auto waitResult = acr->second.wait_for(std::chrono::milliseconds(100));
    if (waitResult == std::future_status::timeout) {
        // timeout
    }
    else {
        veigar::CallResult ret = std::move(acr->second.get());
        if(ret.isSuccess()) {
            std::cout << ret.obj.get().as<std::string>() << std::endl;
        }
        else {
            std::cout << ret.errorMessage << std::endl;
        }
    }
}

vg.releaseCall(acr->first);

//
// 与同步调用相同
// ...
```

与同步调用不同，`asyncCall`函数返回的是`std::shared_ptr<veigar::AsyncCallResult>`，而且调用者在获取到`CallResult`或不再关系调用结果时，需要调用`releaseCall`函数释放资源。

# 5. 性能
使用`examples\echo`程序作为测试用例。

启动A、B、C三个Channel，每个Channel分别使用2个线程向彼此调用100万次，如下图所示：

![3 Channels Test Case](./3-channel-test-case.jpg)

## Windows平台测试结果

测试机器CPU配置：
```txt
12th Gen Intel(R) Core(TM) i7-12700H   2.30 GHz
```

测试结果如下：
```txt
Target channel names (Split by comma):
A,B,C
Async method(0/1):
0
Thread number for each target:
2
Call times each of thread:
1000000
Read/Write Timeout(ms):
100
[Thread 1, Target A] Calling...
[Thread 0, Target C] Calling...
[Thread 0, Target A] Calling...
[Thread 1, Target B] Calling...
[Thread 0, Target B] Calling...
[Thread 1, Target C] Calling...
[Thread 1, Target B] Total 1000000, Success 1000000, Error 0, Used: 59092341us, Average: 59us/call, 16922call/s.

[Thread 0, Target B] Total 1000000, Success 1000000, Error 0, Used: 59112785us, Average: 59us/call, 16916call/s.

[Thread 1, Target A] Total 1000000, Success 1000000, Error 0, Used: 59111520us, Average: 59us/call, 16917call/s.

[Thread 0, Target C] Total 1000000, Success 1000000, Error 0, Used: 59126879us, Average: 59us/call, 16912call/s.

[Thread 0, Target A] Total 1000000, Success 1000000, Error 0, Used: 59206766us, Average: 59us/call, 16889call/s.

[Thread 1, Target C] Total 1000000, Success 1000000, Error 0, Used: 59299407us, Average: 59us/call, 16863call/s.
```

平均每次调用花费59微妙，每秒可以调用约16900次。

![Windows Test Result](./windows-test-result.png)


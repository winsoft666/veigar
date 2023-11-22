# 1. Veigar
Veigar一词来源于英雄联盟游戏里面的“邪恶小法师-维迦”。

![Veigar on LOL](./veigar-lol.jpg)

Veigar是一个跨平台的远程过程调用（RPC）框架。

Veigar基于共享内存技术实现，只支持本机进程或线程间的远程过程调用，这是Veigar与其他RPC框架（如Thrift、grpc）的最大不同之处。

> 据我所知，Veigar是第一个开源的基于共享内存技术的RPC框架。

与其他RPC框架相比，Veigar的优势在于：
- 没有服务端和客户端的概念，每个Veigar实例间都可以相互调用。
- 不用考虑网络端口被占用的问题，也不用考虑端口半关闭状态的问题。
- 在Windows系统上，还可能出现诡异的端口假可用性问题。
- 不用担心由于网络设备异常等原因导致的调用失败。

# 2. 快速上手
虽然Veigar底层是基于msgpack和boost interprocess实现，但我们已经将这两个库包含到项目中，在使用时不需要额外安装这两个库。

Veigar仅支持编译为静态库。

在使用Veigar时，仅仅需要在项目中包含include目录，并引入静态库即可。

## 2.1 同步调用

下面是一个同步调用的示例：

> 本示例为了使代码更加简洁，没有对函数返回值进行校验，请在实际使用中不要这样做！

```cpp
#include <iostream>
#include "veigar/veigar.h"

int main(int argc, char** argv) {
    if(argv != 3) {
        return 1;
    }

    std::string channelName = argv[1];
    std::string targetChannelName = argv[2];

    veigar::Veigar vg;

    vg.init(channelName);

    vg.bind("echo", [](const std::string& msg) {
        std::cout << "RECV:" << msg << std::endl;
        return msg;
    });

    veigar::CallResult ret = vg.syncCall(targetChannelName, "echo", 100, "hello");
    if(ret.isSuccess()) {
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

## 2.2 拒绝异常

我不喜欢异常，因此Veigar也不会通过异常的形式来抛出错误，Veigar会主动捕获所有C++标准库、msgpack、boost异常，以返回值的形式返回给调用者。当调用失败时（`!ret.isSuccess()`)，errorMessage中存储的错误信息就可能是Veigar捕获的异常信息。

## 2.3 异步调用
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

不同于同步调用，`asyncCall`函数返回的是`std::shared_ptr<veigar::AsyncCallResult>`，而且调用者在获取到`CallResult`或不再关系调用结果时，需要调用`releaseCall`函数释放资源。


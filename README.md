[ >>> 简体中文版](README_CN.md)

# Veigar

Veigar is a cross platform remote procedure call (RPC) framework, supports Windows and Linux platforms.

Veigar based on shared memory and only supports RPC between native processes or threads, which is the biggest difference between Veigar and other RPC frameworks such as Thrift and grpc.

> So far as I know, Veigar is the first open source RPC framework based on shared memory.
>
> The term 'Veigar' comes from the 'The Tiny Master of Evil' in League of Legends.

**Features:**

- Expose functions of your program to be called via RPC (from any language implementing msgpack-rpc).

- Call functions through RPC (of programs written in any language).

- No IDL to learn.

- No code generation step to integrate in your build, just C++.

- No concept of server and client, and each Veigar instance can call each other.

- No network, port availability issue, fake availability issues of port on Windows especially.

- Support 3 call methods: Synchronous, Asynchronous with Promise, Asynchronous with Callback.

# Compile

Although Veigar based on msgpack, we have included this library in the project and do not require additional installation when using it.

Although the header file of msgpack is referenced in the public header files of Veigar, it will not contaminate your global msgpack namespace, as the namespace of msgpack in Veigar is `vegar_msgpack`.

CMake can be used for compilation and build, or using [vcpkg](https://github.com/microsoft/vcpkg) to install:

```bash
vcpkg install veigar
```

# Quick Start

When using Veigar, simply include the `include` directory in the project and link Veigar's static library.

## Synchronous Call

Here is an example of synchronous call:

> In order to make the code more concise, this example did not verify the return value of the function. Please do not do this in practical project!

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

Each Veigar instance has a channel name that is unique within the current computer scope. When calling the `init` function, the channel name needs to be specified for the Veigar. Veigar does not detect the uniqueness of the channel, and the caller needs to ensure the uniqueness of the channel name.

In the above example, it is necessary to specify the channel name of the current instance and the channel name of the target instance through command line parameters, such as:

```bash
sample.exe myself other
```

Each instance bind a function named `echo`, which simply returns the msg parameter string as is.

By specifying the `syncCall` function with 'target channel name', 'function name', 'function parameters', and 'timeout milliseconds', the target function can be synchronously called and the call result obtained.

## Asynchronous Call with Promise

Asynchronous call that with promise can be implemented using the `asyncCall` function.

The following is an example of asynchronous call with promise:

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

Unlike synchronous calls, the `asyncCall` function return `std::shared_ptr<AsyncCallResult>`, and the caller needs to call the `releaseCall` function to release resources when obtaining the `CallResult` or when the call result is no longer related.

## Asynchronous Call with Callback

Asynchronous call that with callback also can be implemented using the `asyncCall` function.

The following is an example of asynchronous call with callback:

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

This method does not require calling the `releaseCall` function to release resources.

## RPC function parameter types

Supports regular C++ data types, such as:

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

Also support STL data types, such as:

- std::string
- std::set
- std::vector
- std::map
- std::string_view (C++ 17)
- std::optional (C++ 17)
- Not support std::wstring，but we can use std::vector<uint8_t> to instead of std::wstring

```cpp
 Veigar vg;
 vg.bind("func", [](std::string s, std::vector<std::string>, std::string_view v, std::map<int, bool> m) {
     // ......
 });
```

Veigar can also support custom data types, such as:

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

The detailed parameter binding method can be found in [tests/t_type.cpp](./tests/t_type.cpp)。

# Reject exceptions

I don't like exceptions, so Veigar doesn't throw errors in the form of exceptions. Veigar catch all exceptions of C++ STL and msgpack, and returns them to the caller as return values. 

When the call fails (`!ret.isSuccess()`), the error information stored in the `errorMessage` may be the exception information captured by Veigar.

# Performance

Use the `examples\performance-test` program as a test case.

Process A call process B by use 4 threads and each thread calls 25000 times. On average, it consumes 12 microseconds per call ("calling <--> result").

```txt
Used: 1s240ms721μs, Total: 100000 Success: 100000, Timeout: 0, Failed: 0, Average: 12μs/call.
```

Although there is still a great possibility of performance improvement for Veigar at present, but based on test results, it has far surpassed other RPC frameworks.

[ >>> 简体中文版](README_CN.md)

# Veigar
The term 'Veigar' comes from the game 'The Tiny Master of Evil' in League of Legends.

![Veigar on LOL](./veigar-lol.jpg)

Veigar is a cross platform remote procedure call (RPC) framework. 

Veigar is implemented based on shared memory technology and only supports remote procedure calls between native processes or threads, which is the biggest difference between Veigar and other RPC frameworks such as Thrift and grpc.

> So far as I know, Veigar is the first open source RPC framework based on shared memory technology.

Compared to other RPC frameworks, Veigar's advantages is that:

- Expose functions of your program to be called via RPC (from any language implementing msgpack-rpc).

- Call functions through RPC (of programs written in any language).

- No IDL to learn.

- No code generation step to integrate in your build, just C++.

- No concept of server and client, and each Veigar instance can call each other.

- No network issue, such as being occupied or being semi closed.

- No strange port pseudo availability issues (especially in Windows).

# 2. Compile
Although Veigar's underlying implementation is based on msgpack and boost interprocess, we have included these two libraries in the project and do not require additional installation when using them.

Veigar only supports compiling to static libraries.

CMake can be used for compilation and build, or using [vcpkg](https://github.com/microsoft/vcpkg) to install:

```bash
vcpkg install veigar
```

# 3. Quick Start
Although Veigar's underlying implementation is based on msgpack and boost interprocess, we have included these two libraries in the project and do not require additional installation when using them.

Veigar only supports compiling to static libraries.

When using Veigar, simply include the `include` directory in the project and link Veigar's static library.

## 3.1 Synchronous Call

Here is an example of synchronous call:

> In order to make the code more concise, this example did not verify the return value of the function. Please do not do this in practical use!

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

    veigar::CallResult ret = vg.syncCall(targetChannelName, 100,  "echo", "hello");
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

Each Veigar instance has a channel name that is unique within the current computer scope. When calling the `init` function, the channel name needs to be specified for the Veigar. Veigar does not detect the uniqueness of the channel, and the caller needs to ensure the uniqueness of the channel name.

In the above example, it is necessary to specify the channel name of the current instance and the channel name of the target instance through command line parameters, such as:

```bash
sample.exe myself other
```

Each instance bind a function named `echo`, which simply returns the msg parameter string as is.

By specifying the `syncCall` function with 'target channel name', 'function name', 'function parameters', and 'timeout milliseconds', the target function can be synchronously called and the call result obtained.

## 3.2 Reject exceptions

I don't like exceptions, so Veigar doesn't throw errors in the form of exceptions. Veigar actively catches all C++ standard libraries, msgpack, and boost exceptions, and returns them to the caller as return values. When the call fails (`!ret.isSuccess()`), the error information stored in the `errorMessage` may be the exception information captured by Veigar.

## 3.3 Asynchronous Call

Asynchronous call can be implemented using the `asyncCall` function.

The following is an example of asynchronous call:

```cpp
//
// Same as synchronous call
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
// Same as synchronous call
// ...
```

Unlike synchronous calls, the `asyncCall` function return `std::shared_ptr<veigar::AsyncCallResult>`, and the caller needs to call the `releaseCall` function to release resources when obtaining the `CallResult` or when the call result is no longer related.

# 4. Performance
Veigar is implemented based on shared memory and has the advantages of high throughput and ultra-low latency.

Use the `examples\echo` program for testing.

## 4.1 Single thread
Single thread calls 1 million times (include call function and wait the result), with each call passing ~1050 bytes of parameters. The test results are as follows:

```txt
Target Channel Name:
a2
Async Method(0/1):
0
Thread Number:
1
Call times each of thread:
1000000
Calling...
Total 1000000, Success 1000000, Error 0, Used: 23414ms.
```

Total used 23414ms，Average 42709 calls per second.


## 4.2 Multi-threading

Six threads running simultaneously, with each thread calling 1 million times (include call function and wait the result), and each call passed ~1050 bytes of parameters. The test results are as follows:

```txt
Target Channel Name:
a2
Async Method(0/1):
0
Thread Number:
6
Call times each of thread:
1000000
Calling...
Calling...
Calling...
Calling...
Calling...
Calling...
Total 1000000, Success 1000000, Error 0, Used: 45275ms.

Total 1000000, Success 1000000, Error 0, Used: 45277ms.

Total 1000000, Success 1000000, Error 0, Used: 45295ms.

Total 1000000, Success 1000000, Error 0, Used: 45297ms.

Total 1000000, Success 1000000, Error 0, Used: 45300ms.

Total 1000000, Success 1000000, Error 0, Used: 45313ms.
```

Total used 45275ms，Average 132459 calls per second.

> The above test results may vary on computers with different configurations.
>
> If some multi-threaded mutexes are removed, the performance can be higher, but this requires constraints on the usage of the library, such as having to first `bind` and then `init`. Taking all factors into consideration, Veigar has not removed this part of the mutex, as this performance can already meet the vast majority of usage scenarios.

[ >>> 简体中文版](README_CN.md)

# Veigar
The term 'Veigar' comes from the game 'The Tiny Master of Evil' in League of Legends.

![Veigar on LOL](./veigar-lol.jpg)

Veigar is a cross platform remote procedure call (RPC) framework. 

Veigar is implemented based on shared memory technology and only supports remote procedure calls between native processes or threads, which is the biggest difference between Veigar and other RPC frameworks such as Thrift and grpc.

> So far as I know, Veigar is the first open source RPC framework based on shared memory technology.

Compared to other RPC frameworks, Veigar's advantages is that:

- There is no concept of server and client, and each Veigar instance can call each other.

- There is no need to consider the issue of network ports being occupied, nor the issue of ports being semi closed.

- On Windows systems, there may also be strange port pseudo availability issues.

- Don't worry about call failures caused by network device abnormalities or other reasons.

# 2. Quick Start
Although Veigar's underlying implementation is based on msgpack and boost interprocess, we have included these two libraries in the project and do not require additional installation when using them.

Veigar only supports compiling to static libraries.

When using Veigar, simply include the include directory in the project and introduce a static library.

## 2.1 Synchronous Call

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

Each instance is bound to a function named `echo`, which simply returns the msg parameter string as is.

By specifying the `syncCall` function with 'target channel name', 'function name', 'function parameters', and' timeout milliseconds', the target function can be synchronously called and the call result obtained.

## 2.2 Reject exceptions

I don't like exceptions, so Veigar doesn't throw errors in the form of exceptions. Veigar actively catches all C++ standard libraries, msgpack, and boost exceptions, and returns them to the caller as return values. When the call fails (`!ret.isSuccess()`), the error information stored in the `errorMessage` may be the exception information captured by Veigar.

## 2.3 Asynchronous Call

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
[ >>> 简体中文版](README_CN.md)

# Veigar

Veigar is a cross-platform Remote Procedure Call (RPC) framework designed for Windows and Linux platforms. Unlike traditional RPC frameworks such as Thrift and gRPC, Veigar is built on shared memory technology and exclusively supports inter-process and inter-thread communication.

> To our knowledge, Veigar is the first open-source RPC framework based on shared memory technology.
>
> The name 'Veigar' is inspired by 'The Tiny Master of Evil' character from League of Legends.

**Key Features:**

- Expose program functions for remote invocation via RPC (compatible with any language implementing msgpack-rpc)
- Seamlessly call remote functions across different processes (regardless of implementation language)
- Zero IDL (Interface Definition Language) requirements
- No code generation step required - pure C++ implementation
- Peer-to-peer architecture without traditional client-server roles
- Eliminates network and port availability concerns, particularly addressing Windows-specific port availability issues
- Comprehensive call patterns:
  - Synchronous calls
  - Asynchronous calls with Promise
  - Asynchronous calls with Callback

# Compilation

Veigar includes the msgpack library as part of the project, eliminating the need for additional installation. While the public headers reference msgpack, they are encapsulated within the `veigar_msgpack` namespace to prevent global namespace pollution.

You can build Veigar using either CMake or [vcpkg](https://github.com/microsoft/vcpkg):

```bash
vcpkg install veigar
```

# Quick Start

To use Veigar in your project, simply include the `include` directory and link against Veigar's library.

## Synchronous Calls

The following example demonstrates synchronous RPC calls:

> Note: For brevity, this example omits error checking. In production code, always verify function return values!

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

Each Veigar instance requires a unique channel name within the current computer scope. The channel name is specified during initialization via the `init` function. Note that Veigar does not validate channel name uniqueness - this responsibility lies with the caller.

In the example above, the channel names for both the current and target instances are provided via command-line arguments:

```bash
sample.exe myself other
```

The example demonstrates binding an `echo` function that returns the input message string unchanged.

The `syncCall` function enables synchronous remote function calls by specifying:
- Target channel name
- Function name
- Function parameters
- Timeout duration (in milliseconds)

## Asynchronous Calls with Promise

The `asyncCall` function enables asynchronous calls using promises. Here's an example:

```cpp
std::vector<uint8_t> buf;
std::shared_ptr<AsyncCallResult> acr = vg.asyncCall(targetChannelName, "echo", "hello", 12, 3.14, buf);
if (acr->second.valid()) {
    auto waitResult = acr->second.wait_for(std::chrono::milliseconds(100));
    if (waitResult == std::future_status::timeout) {
        // Handle timeout
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

// Release resources when done
vg.releaseCall(acr->first);
```

Unlike synchronous calls, `asyncCall` returns a `std::shared_ptr<AsyncCallResult>`. The caller must explicitly release resources by calling `releaseCall` when either:
- The `CallResult` has been obtained
- The call result is no longer needed

## Asynchronous Calls with Callback

Veigar also supports asynchronous calls with callbacks using the `asyncCall` function:

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

This approach eliminates the need for explicit resource cleanup via `releaseCall`.

## Supported Parameter Types

Veigar supports a comprehensive range of C++ data types:

### Basic Types
- Boolean: `bool`
- Characters: `char`, `wchar_t`
- Integers: `int`, `unsigned int`, `long`, `unsigned long`, `long long`, `unsigned long long`
- Fixed-width integers: `uint8_t`, `int8_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- Floating-point: `float`, `double`

Example:
```cpp
Veigar vg;
vg.bind("func", [](char c, wchar_t w, int i, int8_t j, int64_t k) {
    // Implementation
});
```

### STL Types
- `std::string`
- `std::set`
- `std::vector`
- `std::map`
- `std::string_view` (C++17)
- `std::optional` (C++17)
- Note: `std::wstring` is not supported; use `std::vector<uint8_t>` as an alternative

Example:
```cpp
Veigar vg;
vg.bind("func", [](std::string s, std::vector<std::string>, std::string_view v, std::map<int, bool> m) {
    // Implementation
});
```

### Custom Types
Custom types can be supported by implementing the msgpack serialization interface:

```cpp
#include "veigar/msgpack/adaptor/define.hpp"

struct MyPoint {
    int x;
    int y;
    MSGPACK_DEFINE(x, y);
};

Veigar vg;
vg.bind("func", [](MyPoint m) {
    // Implementation
});
```

For detailed parameter binding examples, refer to [tests/t_type.cpp](./tests/t_type.cpp).

# Exception Handling

Veigar takes a non-exception approach to error handling. Instead of throwing exceptions, Veigar catches all C++ STL and msgpack exceptions internally and returns them as error messages in the `CallResult`. When a call fails (`!ret.isSuccess()`), the `errorMessage` field contains the captured exception information.

# Performance

Performance testing was conducted using the `examples\performance-test` program:

- Test scenario: Process A calls Process B using 4 threads
- Each thread performs 25,000 calls
- Average call time (including round-trip): 12 microseconds

```txt
Used: 1s240ms721μs, Total: 100000 Success: 100000, Timeout: 0, Failed: 0, Average: 12μs/call.
```

While there is potential for further performance optimization, current test results demonstrate that Veigar significantly outperforms other RPC frameworks.

# Sponsors

We extend our sincere gratitude to all users of this project. Your support and feedback are invaluable to us.

Special thanks to our sponsors:

- sxzxs (https://github.com/sxzxs)

**To support this project, please visit my [GitHub homepage](https://github.com/winsoft666) to make a donation.**
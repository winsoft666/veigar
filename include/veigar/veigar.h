/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_H_
#define VEIGAR_H_
#pragma once

#include <string>
#include <vector>
#include <future>
#include <inttypes.h>
#include "veigar/config.h"
#include "veigar/call_result.h"
#include "veigar/call_dispatcher.h"
#include "veigar/detail/meter.h"

namespace veigar {
class Veigar {
   public:
    Veigar() noexcept;

    Veigar(Veigar&& other) noexcept;

    virtual ~Veigar() noexcept;

    Veigar& operator=(Veigar&& other) noexcept;

    template <typename F>
    bool bind(const std::string& funcName, F func) noexcept;

    void unbind(const std::string& funcName) noexcept;

    // channelName:
    //       Channel name must be unique within the current computer scope.
    //       This uniqueness is guaranteed by the user and Veigar will not check it.
    //
    bool init(const std::string& channelName) noexcept;

    bool isInit() const noexcept;

    void uninit() noexcept;

    std::string channelName() const noexcept;

    std::vector<std::string> bindNames() const noexcept;

    template <typename... Args>
    std::shared_ptr<AsyncCallResult> asyncCall(
        const std::string& targetChannel,
        const std::string& funcName,
        Args... args) noexcept;

    // Release resources.
    // If using 'asyncCall' function, the caller must call the this function to release resources
    //     when obtaining the 'CallResult' or when the call result is no longer related.
    void releaseCall(const std::string& callId) noexcept;

    template <typename... Args>
    CallResult syncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args) noexcept;

    void waitAllResponse() noexcept;

    // Set the timeout for reading and writing shared memory.
    // This timeout is different from the timeout in 'syncCall' function and it same as
    //    the timeout in 'sendMessage' function.
    //
    // Default is 260ms.
    void setReadWriteTimeout(uint32_t ms) noexcept;
    uint32_t readWriteTimeout() const noexcept;

    bool sendMessage(
        const std::string& targetChannel,
        bool toCallQueue,
        const uint8_t* buf,
        size_t bufSize,
        std::string& errMsg) noexcept;

   private:
    std::string getNextCallId(const std::string& funcName) const noexcept;

    // std::promise will not set_exception forever.
    template <typename... Args>
    std::shared_ptr<AsyncCallResult> doAsyncCall(
        const std::string& targetChannel,
        const std::string& funcName,
        Args... args) noexcept;

    bool sendCall(
        const std::string& channelName,
        std::shared_ptr<veigar_msgpack::sbuffer> buffer,
        const std::string& callId,
        const std::string& funcName,
        std::shared_ptr<std::promise<CallResult>> p,
        std::string& errMsg) noexcept;

   private:
    class Impl;
    Impl* impl_ = nullptr;
    std::shared_ptr<detail::CallDispatcher> callDisp_;
};
}  // namespace veigar

#include "veigar.inl"

#endif  // !VEIGAR_H_
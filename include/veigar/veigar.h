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

namespace veigar {

class Veigar {
   public:
    Veigar() noexcept;

    Veigar(Veigar&& other) noexcept;

    virtual ~Veigar() noexcept;

    Veigar& operator=(Veigar&& other) noexcept;

    template <typename F>
    bool bind(const std::string& funcName, F func);

    void unbind(const std::string& funcName);

    // channelName:
    //       Channel name must be unique within the current computer scope.
    //       This uniqueness is guaranteed by the user and Veigar will not check it.
    //
    // msgQueueCapacity:
    //       The maximum number of messages in the message queue.
    //       When push message to the queue, if the total number of messages is greater than this value, the first message will be discarded.
    //
    // expectedMsgMaxSize:
    //       The maximum bytes expected for a single message.
    //       The total shared memory size is msgQueueCapacity * expectedMsgMaxSize.
    //       If the size of a single message is greater than expectedMsgMaxSize, the message can still be sent,
    //       but if the size of a single message is greater than msgQueueCapacity * expectedMsgMaxSize, it will fail.
    //
    bool init(const std::string& channelName, uint32_t msgQueueCapacity = 200, uint32_t expectedMsgMaxSize = 10240);

    bool isInit() const;

    void uninit();

    uint32_t msgQueueCapacity() const;

    uint32_t expectedMsgMaxSize() const;

    std::string channelName() const;

    std::vector<std::string> bindNames() const;

    // timeoutMS: This value should greater than the timeout for setTimeoutOfRWLock.
    //
    template <typename... Args>
    std::shared_ptr<AsyncCallResult> asyncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    template <typename... Args>
    void asyncCall(
        ResultCallback cb,
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    // Release resources.
    // If using 'asyncCall' function with promise, the caller must call the this function to release resources
    //     when obtaining the 'CallResult' or when the call result is no longer related.
    void releaseCall(const std::string& callId);

    template <typename... Args>
    CallResult syncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    // Set the timeout for acquiring the inter-process read-write lock.
    // This timeout is different from the timeout in 'syncCall' or 'asyncCall' functions.
    // This value should greater than the timeout of syncCall / asyncCall.
    //
    // Default is 100ms.
    void setTimeoutOfRWLock(uint32_t ms);
    uint32_t timeoutOfRWLock() const;

   private:
    std::string getNextCallId(const std::string& funcName) const;

    // std::promise will not set_exception forever.
    template <typename... Args>
    std::shared_ptr<AsyncCallResult> doAsyncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    template <typename... Args>
    void doAsyncCallWithCallback(
        ResultCallback cb,
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    bool sendCall(
        const std::string& channelName,
        uint32_t timeoutMS,
        std::shared_ptr<veigar_msgpack::sbuffer> buffer,
        const std::string& callId,
        const std::string& funcName,
        const ResultMeta& retMeta,
        std::string& errMsg);

    bool sendResponse(
        const std::string& targetChannel,
        const uint8_t* buf,
        size_t bufSize,
        std::string& errMsg);

   private:
    class Impl;
    Impl* impl_ = nullptr;
    std::shared_ptr<detail::CallDispatcher> callDisp_;

    friend class detail::CallDispatcher;
};
}  // namespace veigar

#include "veigar.inl"

#endif  // !VEIGAR_H_
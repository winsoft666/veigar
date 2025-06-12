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
/**
 * @brief Veigar class provides inter-process communication capabilities through a message-based RPC system.
 * 
 * This class implements a robust IPC mechanism that allows processes to communicate and invoke
 * functions across process boundaries using a shared memory-based message queue.
 */
class VEIGAR_API Veigar {
   public:
    Veigar() noexcept;

    Veigar(Veigar&& other) noexcept;

    virtual ~Veigar() noexcept;

    Veigar& operator=(Veigar&& other) noexcept;

    /**
     * @brief Binds a function to be callable from other processes
     * @tparam F The type of the function to bind
     * @param funcName The name under which the function will be exposed
     * @param func The function to bind
     * @return true if binding was successful, false otherwise
     */
    template <typename F>
    bool bind(const std::string& funcName, F func);

    /**
     * @brief Unbinds a previously bound function
     * @param funcName The name of the function to unbind
     */
    void unbind(const std::string& funcName);

    /**
     * @brief Initializes the Veigar instance with specified communication parameters
     * 
     * @param channelName A unique identifier for this communication channel within the current computer scope.
     *                   The uniqueness must be guaranteed by the user as Veigar does not validate this.
     * 
     * @param msgQueueCapacity The maximum number of messages that can be queued.
     *                        When this limit is reached, the oldest message will be discarded
     *                        to accommodate new messages.
     * 
     * @param expectedMsgMaxSize The expected maximum size of a single message in bytes.
     *                          The total shared memory allocation will be:
     *                          msgQueueCapacity * expectedMsgMaxSize.
     *                          Messages larger than expectedMsgMaxSize can still be sent,
     *                          but if they exceed msgQueueCapacity * expectedMsgMaxSize,
     *                          the operation will fail.
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init(const std::string& channelName, uint32_t msgQueueCapacity = 200, uint32_t expectedMsgMaxSize = 10240);

    /**
     * @brief Checks if the Veigar instance is properly initialized
     * @return true if initialized, false otherwise
     */
    bool isInit() const;

    /**
     * @brief Uninitializes the Veigar instance and releases associated resources
     */
    void uninit();

    /**
     * @brief Returns the current message queue capacity
     * @return The maximum number of messages that can be queued
     */
    uint32_t msgQueueCapacity() const;

    /**
     * @brief Returns the expected maximum message size
     * @return The maximum size of a single message in bytes
     */
    uint32_t expectedMsgMaxSize() const;

    /**
     * @brief Returns the current channel name
     * @return The unique identifier for this communication channel
     */
    std::string channelName() const;

    /**
     * @brief Returns a list of all currently bound function names
     * @return Vector containing the names of all bound functions
     */
    std::vector<std::string> bindNames() const;

    /**
     * @brief Asynchronously calls a function on a remote process
     * 
     * @tparam Args Variadic template parameter for function arguments
     * @param targetChannel The channel name of the target process
     * @param timeoutMS The maximum time to wait for the response (must be greater than setTimeoutOfRWLock)
     * @param funcName The name of the function to call
     * @param args The arguments to pass to the function
     * @return A shared pointer to an AsyncCallResult object
     */
    template <typename... Args>
    std::shared_ptr<AsyncCallResult> asyncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    /**
     * @brief Asynchronously calls a function on a remote process with a callback
     * 
     * @tparam Args Variadic template parameter for function arguments
     * @param cb The callback function to be executed when the call completes
     * @param targetChannel The channel name of the target process
     * @param timeoutMS The maximum time to wait for the response
     * @param funcName The name of the function to call
     * @param args The arguments to pass to the function
     */
    template <typename... Args>
    void asyncCall(
        ResultCallback cb,
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    /**
     * @brief Releases resources associated with an asynchronous call
     * 
     * This function must be called when using asyncCall with a promise to prevent resource leaks.
     * It should be called either when obtaining the CallResult or when the result is no longer needed.
     * 
     * @param callId The unique identifier of the call to release
     */
    void releaseCall(const std::string& callId);

    /**
     * @brief Synchronously calls a function on a remote process
     * 
     * @tparam Args Variadic template parameter for function arguments
     * @param targetChannel The channel name of the target process
     * @param timeoutMS The maximum time to wait for the response
     * @param funcName The name of the function to call
     * @param args The arguments to pass to the function
     * @return A CallResult object containing the result of the call
     */
    template <typename... Args>
    CallResult syncCall(
        const std::string& targetChannel,
        uint32_t timeoutMS,
        const std::string& funcName,
        Args... args);

    /**
     * @brief Sets the timeout for acquiring inter-process read-write locks
     * 
     * This timeout is distinct from the timeout used in syncCall or asyncCall.
     * It should be set to a value less than the timeout used in those functions.
     * 
     * @param ms The timeout value in milliseconds (default: 30ms)
     */
    void setTimeoutOfRWLock(uint32_t ms);

    /**
     * @brief Returns the current timeout value for read-write locks
     * @return The current timeout value in milliseconds
     */
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
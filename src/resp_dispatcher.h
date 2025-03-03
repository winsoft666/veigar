/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_RESP_DISPATCHER_H_
#define VEIGAR_RESP_DISPATCHER_H_
#pragma once

#include <atomic>
#include <utility>
#include <functional>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include "veigar/config.h"
#include "veigar/msgpack.hpp"
#include "veigar/call_result.h"
#include "semaphore.h"

namespace veigar {
class Veigar;
class MessageQueue;

// Return the response message to the corresponding caller.
class RespDispatcher {
   public:
    RespDispatcher(Veigar* veigar) noexcept;
    ~RespDispatcher() = default;

    bool init();
    bool isInit() const;
    void uninit();

    std::shared_ptr<MessageQueue> messageQueue();

    void addOngoingCall(const std::string& callId, const ResultMeta& retMeta);
    void releaseCall(const std::string& callId);

   private:
    void dispatchRespThreadProc();

   private:
    Veigar* veigar_ = nullptr;
    bool init_ = false;

    std::mutex ongoingCallsMutex_;
    std::unordered_map<std::string, ResultMeta> ongoingCalls_;  // call id -> ResultMeta

    std::vector<std::thread> workers_;

    std::atomic_bool stop_ = { false };
    std::shared_ptr<MessageQueue> respMsgQueue_;
};
}  // namespace veigar

#endif  // !VEIGAR_RESP_DISPATCHER_H_
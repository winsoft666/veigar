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

// Return the response message to the corresponding caller.
class RespDispatcher {
   public:
    RespDispatcher(Veigar* parent) noexcept;
    ~RespDispatcher() noexcept = default;

    bool init();
    bool isInit() const;
    void uninit();

    void pushResp(const std::shared_ptr<veigar_msgpack::object_handle>& respObj);

    void addOngoingCall(const std::string& callId, const ResultMeta& retMeta);
    void releaseCall(const std::string& callId);

   private:
    void dispatchRespThreadProc();

   private:
    Veigar* parent_ = nullptr;
    bool init_ = false;

    std::mutex ongoingCallsMutex_;
    std::unordered_map<std::string, ResultMeta> ongoingCalls_;  // call id -> ResultMeta

    std::vector<std::thread> workers_;
    std::queue<std::shared_ptr<veigar_msgpack::object_handle>> objs_;
    std::mutex objsMutex_;

    std::atomic_bool stop_ = false;
    std::shared_ptr<Semaphore> smh_ = nullptr;
};
}  // namespace veigar

#endif  // !VEIGAR_RESP_DISPATCHER_H_
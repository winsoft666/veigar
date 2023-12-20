/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023 winsoft666 <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
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

class RespDispatcher {
   public:
    RespDispatcher(Veigar* parent) noexcept;
    ~RespDispatcher();

    bool init() noexcept;
    bool isInit() const noexcept;
    void uninit() noexcept;

    void pushResp(const std::shared_ptr<veigar_msgpack::object_handle>& respObj) noexcept;

    void addOngoingCall(const std::string& callId, const std::shared_ptr<std::promise<CallResult>>& cr) noexcept;
    void releaseCall(const std::string& callId) noexcept;
    void waitAllResponse() noexcept;

   private:
    void dispatchRespThreadProc();

   private:
    Veigar* parent_ = nullptr;
    bool init_ = false;

    std::mutex ongoingCallsMutex_;
    std::unordered_map<std::string, std::shared_ptr<std::promise<CallResult>>> ongoingCalls_;  // call id -> call result

    std::vector<std::thread> workers_;
    std::queue<std::shared_ptr<veigar_msgpack::object_handle>> objs_;
    std::mutex objsMutex_;

    std::atomic_bool stop_ = {false};
    std::shared_ptr<Semaphore> smh_ = nullptr;
};
}  // namespace veigar

#endif  // !VEIGAR_RESP_DISPATCHER_H_
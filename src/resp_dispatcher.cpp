/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "resp_dispatcher.h"
#include "veigar/veigar.h"
#include "log.h"
#include "string_helper.h"

namespace veigar {
RespDispatcher::RespDispatcher(Veigar* parent) noexcept :
    parent_(parent) {
    smh_ = std::make_shared<Semaphore>();
}

RespDispatcher::~RespDispatcher() noexcept {
    smh_.reset();
}

bool RespDispatcher::init() noexcept {
    if (init_) {
        return true;
    }

    stop_.store(false);

    if (!smh_->open("")) {
        return false;
    }

    for (size_t i = 0; i < VEIGAR_DISPATCHER_THREAD_NUMBER; ++i) {
        workers_.emplace_back(std::thread(&RespDispatcher::dispatchRespThreadProc, this));
    }

    init_ = true;

    return init_;
}

bool RespDispatcher::isInit() const noexcept {
    return init_;
}

void RespDispatcher::uninit() noexcept {
    if (!init_) {
        return;
    }

    stop_.store(true);

    const size_t releaseNum = workers_.size() * 2;
    for (size_t i = 0; i < releaseNum; i++) {
        smh_->release();
    }

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    ongoingCallsMutex_.lock();
    ongoingCalls_.clear();
    ongoingCallsMutex_.unlock();

    smh_->close();

    init_ = false;
}

void RespDispatcher::pushResp(const std::shared_ptr<veigar_msgpack::object_handle>& respObj) noexcept {
    objsMutex_.lock();
    objs_.emplace(respObj);
    objsMutex_.unlock();

    smh_->release();
}

void RespDispatcher::addOngoingCall(const std::string& callId, const std::shared_ptr<std::promise<CallResult>>& cr) noexcept {
    ongoingCallsMutex_.lock();
    ongoingCalls_[callId] = cr;
    ongoingCallsMutex_.unlock();
}

void RespDispatcher::releaseCall(const std::string& callId) noexcept {
    std::lock_guard<std::mutex> lg(ongoingCallsMutex_);
    auto it = ongoingCalls_.find(callId);
    if (it != ongoingCalls_.cend()) {
        ongoingCalls_.erase(it);
    }
}

void RespDispatcher::dispatchRespThreadProc() {
    std::string exceptionMsg;

    while (true) {
        smh_->wait();
        if (stop_.load()) {
            break;
        }

        std::shared_ptr<veigar_msgpack::object_handle> obj = nullptr;
        do {
            std::lock_guard<std::mutex> lock(objsMutex_);
            if (!objs_.empty()) {
                obj = std::move(objs_.front());
                objs_.pop();
            }
        } while (false);

        if (!obj) {
            continue;
        }

        std::shared_ptr<std::promise<CallResult>> callRetPromise = nullptr;
        CallResult callRet;
        try {
            detail::Response::ResponseMsg r;
            obj->get().convert(r);

            // Check protocol
            uint32_t msgFlag = std::get<0>(r);
            if (msgFlag != 1) {
                exceptionMsg = "Invalid response message flag.";
                veigar::log("Veigar: Error: Invalid response message flag: %d.\n", msgFlag);
                continue;
            }

            std::string callId = std::get<1>(r);
            if (callId.empty()) {
                veigar::log("Veigar: Warning: Call id is empty.\n");
                continue;
            }

            ongoingCallsMutex_.lock();
            if (ongoingCalls_.find(callId) != ongoingCalls_.end()) {
                callRetPromise = ongoingCalls_[callId];
            }
            ongoingCallsMutex_.unlock();

            if (!callRetPromise) {
                veigar::log("Veigar: Warning: Unable to find call: %s.\n", callId.c_str());
                continue;
            }

            auto&& error_obj = std::get<2>(r);
            if (!error_obj.is_nil()) {
                veigar_msgpack::object_handle errObjHandle = veigar_msgpack::clone(error_obj);
                callRet.errorMessage = errObjHandle.get().as<std::string>();
            }

            callRet.obj = std::move(veigar_msgpack::clone(std::get<3>(r)));

            // Last set success.
            callRet.errCode = ErrorCode::SUCCESS;
        } catch (std::exception& e) {
            callRet.errorMessage = StringHelper::StringPrintf("An exception occurred during parsing response message: %s.", e.what());
        } catch (...) {
            callRet.errorMessage = "An exception occurred during parsing response message.";
        }

        if (callRetPromise) {
            callRetPromise->set_value(std::move(callRet));
        }
    }
}

void RespDispatcher::waitAllResponse() noexcept {
    std::lock_guard<std::mutex> lg(ongoingCallsMutex_);
    try {
        for (auto& c : ongoingCalls_) {
            std::shared_ptr<std::promise<CallResult>> p = c.second;
            if (p) {
                auto future = p->get_future();
                if (future.valid()) {
                    future.wait();
                }
            }
        }
    } catch (std::exception& e) {
        veigar::log("Veigar: Warning: An exception occurred during waiting all responses: %s.\n", e.what());
    }
}
}  // namespace veigar
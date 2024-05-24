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
}

bool RespDispatcher::init() {
    if (init_) {
        return true;
    }

    smh_ = std::make_shared<Semaphore>();
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

bool RespDispatcher::isInit() const {
    return init_;
}

void RespDispatcher::uninit() {
    if (!init_) {
        return;
    }

    stop_.store(true);

    if (smh_) {
        const size_t releaseNum = workers_.size() * 2;
        for (size_t i = 0; i < releaseNum; i++) {
            smh_->release();
        }
    }

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    ongoingCallsMutex_.lock();
    ongoingCalls_.clear();
    ongoingCallsMutex_.unlock();

    if (smh_) {
        smh_->close();
        smh_.reset();
    }

    init_ = false;
}

void RespDispatcher::pushResp(const std::shared_ptr<veigar_msgpack::object_handle>& respObj) {
    objsMutex_.lock();
    objs_.emplace(respObj);
    objsMutex_.unlock();

    smh_->release();
}

void RespDispatcher::addOngoingCall(const std::string& callId, const ResultMeta& retMeta) {
    ongoingCallsMutex_.lock();
    ongoingCalls_[callId] = retMeta;
    ongoingCallsMutex_.unlock();
}

void RespDispatcher::releaseCall(const std::string& callId) {
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

        ResultMeta retMeta;
        CallResult callRet;
        std::string callId;
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

            callId = std::get<1>(r);
            if (callId.empty()) {
                veigar::log("Veigar: Warning: Call id is empty.\n");
                continue;
            }

            ongoingCallsMutex_.lock();
            if (ongoingCalls_.find(callId) != ongoingCalls_.end()) {
                retMeta = ongoingCalls_[callId];
            }
            else {
                veigar::log("Veigar: Warning: Unable to find call: %s.\n", callId.c_str());
                ongoingCallsMutex_.unlock();
                continue;
            }
            ongoingCallsMutex_.unlock();

            if (retMeta.metaType != 0 && retMeta.metaType != 1) {
                veigar::log("Veigar: Warning: Invalid result meta type: %d.\n", retMeta.metaType);
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

        if (retMeta.metaType == 0) {
            assert(retMeta.p);
            if (retMeta.p) {
                retMeta.p->set_value(std::move(callRet));
            }
        }
        else if (retMeta.metaType == 1) {
            assert(retMeta.cb);
            if (retMeta.cb) {
                retMeta.cb(callRet);
            }

            releaseCall(callId);
        }
    }
}
}  // namespace veigar
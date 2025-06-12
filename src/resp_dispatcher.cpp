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
#include "message_queue.h"
#include "run_time_recorder.h"

namespace veigar {
RespDispatcher::RespDispatcher(Veigar* veigar) noexcept :
    veigar_(veigar) {
}

bool RespDispatcher::init() {
    if (init_) {
        return true;
    }

    stop_.store(false);

    respMsgQueue_ = std::make_shared<MessageQueue>(veigar_->msgQueueCapacity(), veigar_->expectedMsgMaxSize());
    if (!respMsgQueue_->create(veigar_->channelName() + VEIGAR_RESPONSE_QUEUE_NAME_SUFFIX)) {
        veigar::log("Veigar: [ERROR] Failed to create response message queue for channel: %s.\n", veigar_->channelName().c_str());
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

    for (std::thread& worker : workers_) {
        respMsgQueue_->notifyRead();
    }

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (respMsgQueue_) {
        respMsgQueue_->close();
        respMsgQueue_.reset();
    }

    ongoingCallsMutex_.lock();
    ongoingCalls_.clear();
    ongoingCallsMutex_.unlock();

    init_ = false;
}

std::shared_ptr<veigar::MessageQueue> RespDispatcher::messageQueue() {
    return respMsgQueue_;
}

void RespDispatcher::dispatchRespThreadProc() {
    veigar_msgpack::unpacker respPac;

    try {
        respPac.reserve_buffer(veigar_->expectedMsgMaxSize());
    } catch (std::bad_alloc& e) {
        veigar::log("Veigar: [ERROR] Failed to allocate memory for response buffer (%d bytes): %s.\n", veigar_->expectedMsgMaxSize(), e.what());
        return;
    }

    int64_t written = 0L;
    while (!stop_.load()) {
        written = 0L;
        if (!respMsgQueue_->waitForRead(-1)) {
            continue;
        }

        if (stop_.load()) {
            break;
        }

        if (!respMsgQueue_->processRWLock(veigar_->timeoutOfRWLock())) {
            veigar::log("Veigar: [WARNING] Timeout while acquiring read-write lock for response queue.\n");
            continue;
        }

        const int64_t msgNum = respMsgQueue_->msgNumber();
        if (msgNum <= 0) {
            if (msgNum < 0) {
                veigar::log("Veigar: [ERROR] Failed to query message count from response queue.\n");
            }
            else {
                veigar::log("Veigar: [WARNING] Received read signal for empty response queue.\n");
            }
            respMsgQueue_->processRWUnlock();
            continue;
        }

        if (!respMsgQueue_->popFront(respPac.buffer(), respPac.buffer_capacity(), written)) {
            if (written <= 0) {
                veigar::log("Veigar: [ERROR] Failed to retrieve message from response queue.\n");
                respMsgQueue_->processRWUnlock();
                continue;
            }

            try {
                respPac.reserve_buffer((size_t)written);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: [ERROR] Failed to allocate memory for response buffer (%d bytes): %s.\n", written, e.what());
                respMsgQueue_->processRWUnlock();
                continue;
            }

            if (!respMsgQueue_->popFront(respPac.buffer(), respPac.buffer_capacity(), written)) {
                veigar::log("Veigar: [ERROR] Failed to retrieve message from response queue.\n");
                respMsgQueue_->processRWUnlock();
                continue;
            }
        }

        respPac.buffer_consumed((size_t)written);

        respMsgQueue_->processRWUnlock();

        do {
            veigar_msgpack::object_handle obj;
            bool nextRet = false;
            try {
                nextRet = respPac.next(obj);
            } catch (std::exception& e) {
                veigar::log("Veigar: [ERROR] Exception occurred while parsing response data: %s.\n", e.what());
                nextRet = false;
            } catch (...) {
                veigar::log(
                    "Veigar: [ERROR] Unknown exception occurred while parsing response data. Exception type not derived from std::exception.\n");
                nextRet = false;
            }

            if (!nextRet) {
                break;
            }

            ResultMeta retMeta;
            CallResult callRet;
            std::string callId;
            try {
                detail::Response::ResponseMsg r;
                obj.get().convert(r);

                // Check protocol
                uint32_t msgFlag = std::get<0>(r);
                if (msgFlag != 1) {
                    veigar::log("Veigar: [ERROR] Invalid response message flag: %d.\n", msgFlag);
                    continue;
                }

                callId = std::get<1>(r);
                if (callId.empty()) {
                    veigar::log("Veigar: [WARNING] Call ID is empty.\n");
                    continue;
                }

                ongoingCallsMutex_.lock();
                if (ongoingCalls_.find(callId) != ongoingCalls_.end()) {
                    retMeta = ongoingCalls_[callId];
                }
                else {
                    veigar::log("Veigar: [WARNING] Call not found: %s (Active calls: %d).\n", callId.c_str(), ongoingCalls_.size());
                    ongoingCallsMutex_.unlock();
                    continue;
                }
                ongoingCallsMutex_.unlock();

                if (retMeta.metaType != 0 && retMeta.metaType != 1) {
                    veigar::log("Veigar: [WARNING] Invalid result meta type: %d.\n", retMeta.metaType);
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
        } while (true);  // msgpack unpack while
    }
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
}  // namespace veigar
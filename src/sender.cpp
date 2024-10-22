/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "sender.h"
#include "log.h"
#include "string_helper.h"
#include "veigar/veigar.h"
#include "time_util.h"
#include "run_time_recorder.h"

namespace veigar {
Sender::Sender(Veigar* v) noexcept :
    veigar_(v) {
}

bool Sender::init(std::shared_ptr<RespDispatcher> respDisp,
                  std::shared_ptr<MessageQueue> selfCallMQ,
                  std::shared_ptr<MessageQueue> selfRespMQ) {
    if (isInit_)
        return true;

    stopEvent_.reset();

    respDisp_ = respDisp;
    selfCallMQ_ = selfCallMQ;
    selfRespMQ_ = selfRespMQ;

    for (size_t i = 0; i < VEIGAR_SEND_CALL_THREAD_NUMBER; ++i) {
        callWorkers_.emplace_back(std::thread(&Sender::sendCallThreadProc, this));
    }

    for (size_t i = 0; i < VEIGAR_SEND_RESPONSE_THREAD_NUMBER; ++i) {
        respWorkers_.emplace_back(std::thread(&Sender::sendRespThreadProc, this));
    }

    isInit_ = true;

    return true;
}

void Sender::uninit() {
    stopEvent_.set();

    callListSetEvent_.cancel();
    respListSetEvent_.cancel();

    for (std::thread& w : callWorkers_) {
        if (w.joinable()) {
            w.join();
        }
    }

    for (std::thread& w : respWorkers_) {
        if (w.joinable()) {
            w.join();
        }
    }

    targetCallMQsMutex_.lock();
    for (auto it : targetCallMsgQueues_) {
        if (it.second) {
            it.second->close();
        }
    }
    targetCallMsgQueues_.clear();
    targetCallMQsMutex_.unlock();

    targetRespMQsMutex_.lock();
    for (auto it : targetRespMsgQueues_) {
        if (it.second) {
            it.second->close();
        }
    }
    targetRespMsgQueues_.clear();
    targetRespMQsMutex_.unlock();

    // release all calls memory
    callListMutex_.lock();
    while (!callList_.empty()) {
        CallMeta cm = callList_.front();
        if (cm.data) {
            free(cm.data);
        }
        callList_.pop();
    }
    callListMutex_.unlock();

    // release all responses memory
    respListMutex_.lock();
    while (!callList_.empty()) {
        RespMeta rm = respList_.front();
        if (rm.data) {
            free(rm.data);
        }
        respList_.pop();
    }
    respListMutex_.unlock();

    respDisp_.reset();
    selfCallMQ_.reset();
    selfRespMQ_.reset();

    isInit_ = false;
}

bool Sender::isInit() const {
    return isInit_;
}

void Sender::addCall(const Sender::CallMeta& cm) {
    callListMutex_.lock();
    callList_.emplace(cm);
    callListMutex_.unlock();

    callListSetEvent_.set();
}

void Sender::addResp(const Sender::RespMeta& rm) {
    respListMutex_.lock();
    respList_.emplace(rm);
    respListMutex_.unlock();

    respListSetEvent_.set();
}

std::shared_ptr<MessageQueue> Sender::getTargetCallMessageQueue(const std::string& channelName) {
    std::lock_guard<std::mutex> lg(targetCallMQsMutex_);
    std::shared_ptr<MessageQueue> queue = nullptr;
    auto it = targetCallMsgQueues_.find(channelName);
    if (it != targetCallMsgQueues_.cend()) {
        return it->second;
    }

    queue = std::make_shared<MessageQueue>(veigar_->msgQueueCapacity(), veigar_->expectedMsgMaxSize());
    if (!queue->open(channelName + VEIGAR_CALL_QUEUE_NAME_SUFFIX)) {
        queue.reset();
        veigar::log("Veigar: Error: Open call message queue(%s) failed.\n", channelName.c_str());
        return nullptr;
    }

    targetCallMsgQueues_[channelName] = queue;
    return queue;
}

std::shared_ptr<MessageQueue> Sender::getTargetRespMessageQueue(const std::string& channelName) {
    std::lock_guard<std::mutex> lg(targetRespMQsMutex_);
    std::shared_ptr<MessageQueue> queue = nullptr;
    auto it = targetRespMsgQueues_.find(channelName);
    if (it != targetRespMsgQueues_.cend()) {
        return it->second;
    }

    queue = std::make_shared<MessageQueue>(veigar_->msgQueueCapacity(), veigar_->expectedMsgMaxSize());
    if (!queue->open(channelName + VEIGAR_RESPONSE_QUEUE_NAME_SUFFIX)) {
        queue.reset();
        veigar::log("Veigar: Error: Open response message queue(%s) failed.\n", channelName.c_str());
        return nullptr;
    }

    targetRespMsgQueues_[channelName] = queue;
    return queue;
}

void Sender::sendCallThreadProc() {
    std::string errMsg;
    while (true) {
        if (!callListSetEvent_.wait(30))
            continue;

        if (callListSetEvent_.isCancelled())
            break;

        callListSetEvent_.unset();

        for (;;) {
            if (callListSetEvent_.isCancelled())
                break;

            CallMeta cm;
            callListMutex_.lock();
            if (callList_.empty()) {
                callListMutex_.unlock();
                break;
            }
            cm = callList_.front();
            callList_.pop();
            callListMutex_.unlock();

            respDisp_->addOngoingCall(cm.callId, cm.resultMeta);

            ErrorCode ec = ErrorCode::FAILED;
            std::shared_ptr<MessageQueue> mq = nullptr;
            try {
                errMsg.clear();

                if (cm.channel == veigar_->channelName()) {
                    mq = selfCallMQ_;
                }
                else {
                    mq = getTargetCallMessageQueue(cm.channel);
                }

                if (mq) {
                    if (mq->rwLock(veigar_->timeoutOfRWLock())) {
                        if (checkSpaceAndWait(mq, cm.dataSize, cm.startCallTimePoint, cm.timeout)) {
                            if (mq->pushBack(cm.data, cm.dataSize)) {
                                ec = ErrorCode::SUCCESS;
                            }
                            else {
                                errMsg = "Unable to push message to queue.";
                            }
                        }
                        else {
                            ec = ErrorCode::TIMEOUT;
                            errMsg = "Waiting for call queue availability timeout.";
                        }
                        mq->rwUnlock();
                    }
                    else {
                        ec = ErrorCode::TIMEOUT;
                        errMsg = "Get rw-lock timeout when push call.";
                    }
                }
                else {
                    errMsg = "Unable to get target message queue. It seems that the channel not started.";
                }

            } catch (std::exception& e) {
                if (mq) {
                    mq->rwUnlock();  // always try to unlock again
                }
                veigar::log("Veigar: Error: An exception occurred during pushing message to call queue: %s.\n", e.what());
                errMsg = StringHelper::StringPrintf("An exception occurred during pushing message to call queue: %s.", e.what());
            } catch (...) {
                if (mq) {
                    mq->rwUnlock();  // always try to unlock again
                }
                veigar::log("Veigar: Error: An exception occurred during pushing message to call queue.\n");
                errMsg = "An exception occurred during pushing message to call queue.";
            }

            if (ec != ErrorCode::SUCCESS) {
                respDisp_->releaseCall(cm.callId);

                CallResult failedRet;
                failedRet.errCode = ec;
                failedRet.errorMessage = errMsg;

                if (cm.resultMeta.metaType == 0) {
                    if (cm.resultMeta.p) {
                        cm.resultMeta.p->set_value(std::move(failedRet));
                    }
                }
                else if (cm.resultMeta.metaType == 1) {
                    if (cm.resultMeta.cb) {
                        cm.resultMeta.cb(failedRet);
                    }
                }
            }

            if (cm.data) {
                free(cm.data);
            }
        }
    }
}

void Sender::sendRespThreadProc() {
    std::string errMsg;
    while (true) {
        if (!respListSetEvent_.wait(30))
            continue;

        if (respListSetEvent_.isCancelled())
            break;

        respListSetEvent_.unset();

        for (;;) {
            if (respListSetEvent_.isCancelled())
                break;

            RespMeta rm;
            respListMutex_.lock();
            if (respList_.empty()) {
                respListMutex_.unlock();
                break;
            }
            rm = respList_.front();
            respList_.pop();
            respListMutex_.unlock();

            ErrorCode ec = ErrorCode::FAILED;
            std::shared_ptr<MessageQueue> mq = nullptr;
            try {
                errMsg.clear();

                if (rm.channel == veigar_->channelName()) {
                    mq = selfRespMQ_;
                }
                else {
                    mq = getTargetRespMessageQueue(rm.channel);
                }

                if (mq) {
                    if (mq->rwLock(veigar_->timeoutOfRWLock())) {
                        if (checkSpaceAndWait(mq, rm.dataSize, rm.startCallTimePoint, rm.timeout)) {
                            if (mq->pushBack(rm.data, rm.dataSize)) {
                                ec = ErrorCode::SUCCESS;
                            }
                            else {
                                errMsg = "Unable to push message to response queue.";
                            }
                        }
                        else {
                            ec = ErrorCode::TIMEOUT;
                            errMsg = "Waiting for response queue availability timeout.";
                        }
                        mq->rwUnlock();
                    }
                    else {
                        ec = ErrorCode::TIMEOUT;
                        errMsg = "Get rw-lock timeout when push response.";
                    }
                }
                else {
                    errMsg = "Unable to get target message queue. It seems that the channel not started.";
                }

            } catch (std::exception& e) {
                if (mq) {
                    mq->rwUnlock();  // always try to unlock again
                }
                errMsg = StringHelper::StringPrintf("An exception occurred during pushing message to response queue: %s.", e.what());
            } catch (...) {
                if (mq) {
                    mq->rwUnlock();  // always try to unlock again
                }
                errMsg = "An exception occurred during parsing pushing message to response queue.";
            }

            if (ec != ErrorCode::SUCCESS) {
                veigar::log("Veigar: Error: Send response failed: %s\n", errMsg.c_str());
            }

            if (rm.data) {
                free(rm.data);
            }
        }
    }
}

bool Sender::checkSpaceAndWait(std::shared_ptr<MessageQueue> mq,
                               int64_t needSize,
                               int64_t startCallTimePoint,
                               int64_t timeout) {
    bool result = false;
    do {
        bool waitable = false;
        if (mq->checkSpaceSufficient(needSize, waitable)) {
            result = true;
            break;
        }

        if (!waitable) {
            break;
        }

        int64_t used = TimeUtil::GetCurrentTimestamp() - startCallTimePoint;
        if (used >= timeout) {
            break;
        }

        int64_t waitMS = 0;
        int64_t ms = (timeout - used) / 1000;
        if (ms > 5) {
            waitMS = 5;
        }
        else {
            waitMS = ms;
        }

        if (stopEvent_.wait(waitMS)) {
            break;  // user call uninit, exit now!
        }
    } while (true);

    return result;
}
}  // namespace veigar

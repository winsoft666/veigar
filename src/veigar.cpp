/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "veigar/veigar.h"
#include "uuid.h"
#include "log.h"
#include "message_queue.h"
#include "time_util.h"
#include "resp_dispatcher.h"

namespace veigar {
const std::string kCallQueueSuffix = "_call";
const std::string kRespQueueSuffix = "_resp";

class Veigar::Impl {
   public:
    Impl(Veigar* parent) :
        parent_(parent) {
        respDispatcher_ = std::make_shared<RespDispatcher>(parent_);
    }

    ~Impl() {
        respDispatcher_.reset();
    }

    bool init(const std::string& channelName, uint32_t msgQueueCapacity, uint32_t expectedMsgMaxSize) {
        if (isInit_) {
            veigar::log("Veigar: Warning: Already init.\n");
            if (channelName_ == channelName) {
                return true;
            }
            return false;
        }

        do {
            quit_.store(false);

            if (channelName.empty()) {
                veigar::log("Veigar: Error: Channel name is empty.\n");
                break;
            }

            channelName_ = channelName;
            msgQueueCapacity_ = msgQueueCapacity;
            expectedMsgMaxSize_ = expectedMsgMaxSize;

            uuid_ = UUID::Create();
            if (uuid_.empty()) {
                veigar::log("Veigar: Error: Generate uuid failed.\n");
                break;
            }

            assert(parent_->callDisp_);
            if (!parent_->callDisp_->init()) {
                veigar::log("Veigar: Error: Init call dispatcher failed.\n");
                break;
            }

            assert(respDispatcher_);
            if (!respDispatcher_->init()) {
                veigar::log("Veigar: Error: Init response dispatcher failed.\n");
                break;
            }

            callMsgQueue_ = std::make_shared<MessageQueue>(true, msgQueueCapacity, expectedMsgMaxSize);
            if (!callMsgQueue_->create(channelName_ + kCallQueueSuffix)) {
                veigar::log("Veigar: Error: Create call message queue(%s) failed.\n", channelName_.c_str());
                break;
            }

            respMsgQueue_ = std::make_shared<MessageQueue>(true, msgQueueCapacity, expectedMsgMaxSize);
            if (!respMsgQueue_->create(channelName_ + kRespQueueSuffix)) {
                veigar::log("Veigar: Error: Create response message queue(%s) failed.\n", channelName_.c_str());
                break;
            }

            recvCallBufSize_ = expectedMsgMaxSize;
            recvRespBufSize_ = expectedMsgMaxSize;

            try {
                callPac_.reserve_buffer(recvCallBufSize_);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: Error: Pre-alloc call memory(%d bytes) failed: %s.\n", recvCallBufSize_, e.what());
                break;
            }

            try {
                respPac_.reserve_buffer(recvRespBufSize_);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: Error: Pre-alloc response memory(%d bytes) failed: %s.\n", recvRespBufSize_, e.what());
                break;
            }

            try {
                recvCallThread_ = std::async(std::launch::async, &Impl::RecvCallThreadProc, this);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during starting receive call thread: %s.\n", e.what());
                break;
            }

            try {
                recvRespThread_ = std::async(std::launch::async, &Impl::RecvReponseThreadProc, this);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during starting receive response thread: %s.\n", e.what());
                break;
            }
            isInit_ = true;
        } while (false);

        if (!isInit_) {
            quit_.store(true);

            if (callMsgQueue_) {
                callMsgQueue_->notifyRead();
            }

            if (respMsgQueue_) {
                respMsgQueue_->notifyRead();
            }

            if (recvCallThread_.valid()) {
                recvCallThread_.wait();
            }

            if (recvRespThread_.valid()) {
                recvRespThread_.wait();
            }

            if (callMsgQueue_) {
                callMsgQueue_->close();
                callMsgQueue_.reset();
            }

            if (respMsgQueue_) {
                respMsgQueue_->close();
                respMsgQueue_.reset();
            }

            if (parent_->callDisp_->isInit()) {
                parent_->callDisp_->uninit();
            }

            if (respDispatcher_->isInit()) {
                respDispatcher_->uninit();
            }

            uuid_.clear();
            channelName_.clear();
        }
        else {
            veigar::log("Veigar: Init success, channel: %s, uuid: %s.\n", channelName_.c_str(), uuid_.c_str());
        }

        return isInit_;
    }

    void uninit() {
        if (!isInit_) {
            return;
        }

        quit_.store(true);

        if (callMsgQueue_) {
            callMsgQueue_->notifyRead();
        }

        if (respMsgQueue_) {
            respMsgQueue_->notifyRead();
        }

        if (recvCallThread_.valid()) {
            recvCallThread_.wait();
        }

        if (recvRespThread_.valid()) {
            recvRespThread_.wait();
        }

        if (callMsgQueue_) {
            callMsgQueue_->close();
            callMsgQueue_.reset();
        }

        if (respMsgQueue_) {
            respMsgQueue_->close();
            respMsgQueue_.reset();
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

        assert(parent_ && parent_->callDisp_);
        if (parent_->callDisp_->isInit()) {
            parent_->callDisp_->uninit();
        }

        assert(respDispatcher_);
        if (respDispatcher_->isInit()) {
            respDispatcher_->uninit();
        }

        isInit_ = false;
    }

    std::shared_ptr<MessageQueue> getTargetCallMessageQueue(const std::string& channelName) {
        std::lock_guard<std::mutex> lg(targetCallMQsMutex_);
        std::shared_ptr<MessageQueue> queue = nullptr;
        auto it = targetCallMsgQueues_.find(channelName);
        if (it != targetCallMsgQueues_.cend()) {
            return it->second;
        }

        queue = std::make_shared<MessageQueue>(true, msgQueueCapacity_, expectedMsgMaxSize_);
        if (!queue->open(channelName + kCallQueueSuffix)) {
            queue.reset();
            veigar::log("Veigar: Error: Open call message queue(%s) failed.\n", channelName.c_str());
            return nullptr;
        }

        targetCallMsgQueues_[channelName] = queue;
        return queue;
    }

    std::shared_ptr<MessageQueue> getTargetRespMessageQueue(const std::string& channelName) {
        std::lock_guard<std::mutex> lg(targetRespMQsMutex_);
        std::shared_ptr<MessageQueue> queue = nullptr;
        auto it = targetRespMsgQueues_.find(channelName);
        if (it != targetRespMsgQueues_.cend()) {
            return it->second;
        }

        queue = std::make_shared<MessageQueue>(true, msgQueueCapacity_, expectedMsgMaxSize_);
        if (!queue->open(channelName + kRespQueueSuffix)) {
            queue.reset();
            veigar::log("Veigar: Error: Open response message queue(%s) failed.\n", channelName.c_str());
            return nullptr;
        }

        targetRespMsgQueues_[channelName] = queue;
        return queue;
    }

    bool sendMessage(const std::string& channelName,
                     bool isCall,
                     const uint8_t* buf,
                     size_t bufSize,
                     std::string& errMsg) noexcept {
        try {
            std::shared_ptr<MessageQueue> mq = nullptr;
            if (channelName == channelName_) {
                mq = isCall ? callMsgQueue_ : respMsgQueue_;
            }
            else {
                mq = isCall ? getTargetCallMessageQueue(channelName) : getTargetRespMessageQueue(channelName);
            }

            if (!mq) {
                errMsg = "Unable to get target message queue. It seems that the channel not started.";
                return false;
            }

            if (!mq->pushBack(rwTimeout_.load(), buf, bufSize)) {
                errMsg = "Unable to push message to queue.";
                return false;
            }

            return true;
        } catch (std::exception& e) {
            veigar::log("Veigar: Error: An exception occurred during sending message: %s.\n", e.what());
            errMsg = e.what();
            return false;
        }
    }

    void RecvCallThreadProc() {
        void* recvBuf = malloc(recvCallBufSize_);
        if (!recvBuf) {
            veigar::log("Veigar: Error: Allocate receive call buffer(%d bytes) failed.\n", recvCallBufSize_);
            return;
        }

        int64_t written = 0L;
        while (!quit_.load()) {
            written = 0L;
            if (!callMsgQueue_->wait(-1)) {
                continue;
            }

            if (quit_.load()) {
                break;
            }

            if (!callMsgQueue_->popFront(rwTimeout_.load(), recvBuf, recvCallBufSize_, written)) {
                if (written <= 0) {
                    continue;
                }

                if (recvBuf) {
                    free(recvBuf);
                    recvBuf = nullptr;
                    recvCallBufSize_ = 0;
                }

                recvBuf = malloc((size_t)written);
                if (!recvBuf) {
                    veigar::log("Veigar: Error: Buffer size too small and reallocate %" PRId64 " bytes failed.\n", written);
                    continue;
                }

                recvCallBufSize_ = (uint32_t)written;
                if (!callMsgQueue_->popFront(rwTimeout_.load(), recvBuf, recvCallBufSize_, written)) {
                    continue;
                }
            }

            handleCallMessage(recvBuf, written);
        }
    }

    void handleCallMessage(void* msg, int64_t msgSize) {
        assert(callPac_.buffer());
        callPac_.reserve_buffer((size_t)msgSize);
        memcpy(callPac_.buffer(), msg, (size_t)msgSize);
        callPac_.buffer_consumed((size_t)msgSize);

        do {
            std::shared_ptr<veigar_msgpack::object_handle> result = std::make_shared<veigar_msgpack::object_handle>();

            bool nextRet = false;
            try {
                veigar_msgpack::object_handle& objRef = *result;
                nextRet = callPac_.next(objRef);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during parsing received call data: %s.\n", e.what());
                nextRet = false;
            } catch (...) {
                veigar::log(
                    "Veigar: Error: An exception occurred during parsing received call data. The exception is not derived from std::exception. "
                    "No further information available.\n");
                nextRet = false;
            }

            if (!nextRet) {
                break;
            }

            assert(parent_ && parent_->callDisp_);
            if (parent_->callDisp_) {
                parent_->callDisp_->pushCall(result);
            }
        } while (true);
    }

    void RecvReponseThreadProc() {
        void* recvBuf = malloc(recvRespBufSize_);
        if (!recvBuf) {
            veigar::log("Veigar: Error: Allocate receive response buffer(%d bytes) failed.\n", recvRespBufSize_);
            return;
        }

        int64_t written = 0L;
        while (!quit_.load()) {
            written = 0L;
            if (!respMsgQueue_->wait(-1)) {
                continue;
            }

            if (quit_.load()) {
                break;
            }

            if (!respMsgQueue_->popFront(rwTimeout_.load(), recvBuf, recvRespBufSize_, written)) {
                if (written <= 0) {
                    continue;
                }

                if (recvBuf) {
                    free(recvBuf);
                    recvBuf = nullptr;
                    recvRespBufSize_ = 0;
                }

                recvBuf = malloc((size_t)written);
                if (!recvBuf) {
                    veigar::log("Veigar: Error: Buffer size too small and reallocate %" PRId64 " bytes failed.\n", written);
                    continue;
                }

                recvRespBufSize_ = (uint32_t)written;
                if (!respMsgQueue_->popFront(rwTimeout_.load(), recvBuf, recvRespBufSize_, written)) {
                    continue;
                }
            }

            handleReponseMessage(recvBuf, written);
        }
    }

    void handleReponseMessage(void* msg, int64_t msgSize) {
        assert(respPac_.buffer());
        respPac_.reserve_buffer((size_t)msgSize);
        memcpy(respPac_.buffer(), msg, (size_t)msgSize);
        respPac_.buffer_consumed((size_t)msgSize);

        do {
            std::shared_ptr<veigar_msgpack::object_handle> result = std::make_shared<veigar_msgpack::object_handle>();

            bool nextRet = false;
            try {
                veigar_msgpack::object_handle& objRef = *result;
                nextRet = respPac_.next(objRef);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during parsing received data: %s.\n", e.what());
                nextRet = false;
            } catch (...) {
                veigar::log(
                    "Veigar: Error: An exception occurred during parsing received data. The exception is not derived from std::exception. "
                    "No further information available.\n");
                nextRet = false;
            }

            if (!nextRet) {
                break;
            }

            assert(respDispatcher_);
            if (respDispatcher_) {
                respDispatcher_->pushResp(result);
            }
        } while (true);
    }

    Veigar* parent_ = nullptr;
    std::atomic_bool quit_ = {false};
    bool isInit_ = false;
    uint32_t msgQueueCapacity_ = 0;
    uint32_t expectedMsgMaxSize_ = 0;
    uint32_t recvCallBufSize_ = 0;
    uint32_t recvRespBufSize_ = 0;

    std::atomic<uint32_t> rwTimeout_ = {260};  // ms

    std::atomic<uint32_t> callIndex_ = {0};
    std::string channelName_;
    std::string uuid_;

    std::shared_ptr<MessageQueue> callMsgQueue_;
    std::shared_ptr<MessageQueue> respMsgQueue_;

    std::mutex targetCallMQsMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> targetCallMsgQueues_;

    std::mutex targetRespMQsMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> targetRespMsgQueues_;

    std::shared_future<void> recvCallThread_;
    std::shared_future<void> recvRespThread_;
    veigar_msgpack::unpacker callPac_;
    veigar_msgpack::unpacker respPac_;

    std::shared_ptr<RespDispatcher> respDispatcher_;
};

Veigar::Veigar() noexcept :
    impl_(new Veigar::Impl(this)) {
    callDisp_ = std::make_shared<detail::CallDispatcher>(this);
}

Veigar::Veigar(Veigar&& other) noexcept {
    *this = std::move(other);
}

Veigar& Veigar::operator=(Veigar&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
        other.impl_ = nullptr;
    }
    return *this;
}

Veigar::~Veigar() noexcept {
    assert(!isInit());
    if (isInit()) {
        uninit();
    }
    if (callDisp_) {
        callDisp_.reset();
    }

    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

bool Veigar::init(const std::string& channelName, uint32_t msgQueueCapacity, uint32_t expectedMsgMaxSize) {
    assert(impl_);
    return impl_->init(channelName, msgQueueCapacity, expectedMsgMaxSize);
}

bool Veigar::isInit() const {
    assert(impl_);
    return impl_->isInit_;
}

void Veigar::uninit() {
    assert(impl_);
    impl_->uninit();
}

std::string Veigar::channelName() const {
    assert(impl_);
    return impl_->channelName_;
}

void Veigar::unbind(const std::string& funcName) {
    if (callDisp_) {
        callDisp_->unbind(funcName);
    }
}

std::vector<std::string> Veigar::bindNames() const {
    if (!callDisp_) {
        return {};
    }
    return callDisp_->names();
}

bool Veigar::sendMessage(const std::string& targetChannel,
                         bool toCallQueue,
                         const uint8_t* buf,
                         size_t bufSize,
                         std::string& errMsg) {
    assert(impl_);
    return impl_->sendMessage(targetChannel, toCallQueue, buf, bufSize, errMsg);
}

void Veigar::setReadWriteTimeout(uint32_t ms) {
    assert(impl_);
    if (ms > 0) {
        impl_->rwTimeout_.store(ms);
    }
}

uint32_t Veigar::readWriteTimeout() const {
    assert(impl_);
    return impl_->rwTimeout_.load();
}

std::string Veigar::getNextCallId(const std::string& funcName) const {
    assert(impl_);
    uint32_t idx = impl_->callIndex_.fetch_add(1);
    return (impl_->uuid_ + "_" + funcName + "_" + std::to_string(idx));
}

bool Veigar::sendCall(const std::string& channelName,
                      std::shared_ptr<veigar_msgpack::sbuffer> buffer,
                      const std::string& callId,
                      const std::string& funcName,
                      const ResultMeta& retMeta,
                      std::string& exceptionMsg) {
    assert(impl_);
    if (!impl_->respDispatcher_) {
        return false;
    }

    impl_->respDispatcher_->addOngoingCall(callId, retMeta);

    return impl_->sendMessage(channelName,
                              true,
                              (const uint8_t*)buffer->data(),
                              buffer->size(),
                              exceptionMsg);
}

void Veigar::releaseCall(const std::string& callId) {
    if (impl_->respDispatcher_) {
        impl_->respDispatcher_->releaseCall(callId);
    }
}

}  // namespace veigar

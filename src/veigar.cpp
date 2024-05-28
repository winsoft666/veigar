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
#include "sender.h"

namespace veigar {
class Veigar::Impl {
   public:
    Impl(Veigar* veigar) noexcept :
        veigar_(veigar) {
    }

    ~Impl() noexcept = default;

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

            respDispatcher_ = std::make_shared<RespDispatcher>(veigar_);
            sender_ = std::make_shared<Sender>(veigar_);

            assert(veigar_->callDisp_);
            if (!veigar_->callDisp_->init()) {
                veigar::log("Veigar: Error: Init call dispatcher failed.\n");
                break;
            }

            assert(respDispatcher_);
            if (!respDispatcher_->init()) {
                veigar::log("Veigar: Error: Init response dispatcher failed.\n");
                break;
            }

            callMsgQueue_ = std::make_shared<MessageQueue>(true, msgQueueCapacity, expectedMsgMaxSize);
            if (!callMsgQueue_->create(channelName_ + VEIGAR_CALL_QUEUE_NAME_SUFFIX)) {
                veigar::log("Veigar: Error: Create call message queue(%s) failed.\n", channelName_.c_str());
                break;
            }

            respMsgQueue_ = std::make_shared<MessageQueue>(true, msgQueueCapacity, expectedMsgMaxSize);
            if (!respMsgQueue_->create(channelName_ + VEIGAR_RESPONSE_QUEUE_NAME_SUFFIX)) {
                veigar::log("Veigar: Error: Create response message queue(%s) failed.\n", channelName_.c_str());
                break;
            }

            assert(sender_);
            if (!sender_->init(respDispatcher_, callMsgQueue_, respMsgQueue_)) {
                veigar::log("Veigar: Error: Init sender failed.\n");
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
                handleCallThread_ = std::async(std::launch::async, &Impl::HandleCallThreadProc, this);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during starting receive call thread: %s.\n", e.what());
                break;
            }

            try {
                handleRespThread_ = std::async(std::launch::async, &Impl::HandleReponseThreadProc, this);
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

            if (handleCallThread_.valid()) {
                handleCallThread_.wait();
            }

            if (handleRespThread_.valid()) {
                handleRespThread_.wait();
            }

            if (callMsgQueue_) {
                callMsgQueue_->close();
                callMsgQueue_.reset();
            }

            if (respMsgQueue_) {
                respMsgQueue_->close();
                respMsgQueue_.reset();
            }

            if (veigar_->callDisp_->isInit()) {
                veigar_->callDisp_->uninit();
            }

            if (respDispatcher_->isInit()) {
                respDispatcher_->uninit();
            }

            if (sender_->isInit()) {
                sender_->uninit();
            }

            uuid_.clear();
            channelName_.clear();

            sender_.reset();
            respDispatcher_.reset();
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

        if (handleCallThread_.valid()) {
            handleCallThread_.wait();
        }

        if (handleRespThread_.valid()) {
            handleRespThread_.wait();
        }

        if (callMsgQueue_) {
            callMsgQueue_->close();
            callMsgQueue_.reset();
        }

        if (respMsgQueue_) {
            respMsgQueue_->close();
            respMsgQueue_.reset();
        }

        assert(sender_);
        if (sender_) {
            if (sender_->isInit()) {
                sender_->uninit();
            }
            sender_.reset();
        }

        assert(veigar_ && veigar_->callDisp_);
        if (veigar_->callDisp_->isInit()) {
            veigar_->callDisp_->uninit();
        }

        assert(respDispatcher_);
        if (respDispatcher_) {
            if (respDispatcher_->isInit()) {
                respDispatcher_->uninit();
            }
            respDispatcher_.reset();
        }

        isInit_ = false;
    }

    void HandleCallThreadProc() {
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

            if (!callMsgQueue_->rwLock(rwTimeout_.load())) {
                veigar::log("Veigar: Warning: Get rw-lock timeout when pop front from call message queue.\n");
                continue;
            }

            if (!callMsgQueue_->popFront(recvBuf, recvCallBufSize_, written)) {
                if (written <= 0) {
                    callMsgQueue_->rwUnlock();
                    continue;
                }

                if (recvBuf) {
                    free(recvBuf);
                    recvBuf = nullptr;
                    recvCallBufSize_ = 0;
                }

                recvBuf = malloc((size_t)written);
                if (!recvBuf) {
                    callMsgQueue_->rwUnlock();
                    veigar::log("Veigar: Error: Buffer size too small and reallocate %" PRId64 " bytes failed.\n", written);
                    continue;
                }

                recvCallBufSize_ = (uint32_t)written;
                if (!callMsgQueue_->popFront(recvBuf, recvCallBufSize_, written)) {
                    callMsgQueue_->rwUnlock();
                    continue;
                }
            }

            callMsgQueue_->rwUnlock();

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

            assert(veigar_ && veigar_->callDisp_);
            if (veigar_->callDisp_) {
                veigar_->callDisp_->pushCall(result);
            }
        } while (true);
    }

    void HandleReponseThreadProc() {
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

            if (!respMsgQueue_->rwLock(rwTimeout_.load())) {
                veigar::log("Veigar: Warning: Get rw-lock timeout when pop front from response message queue.\n");
                continue;
            }

            if (!respMsgQueue_->popFront(recvBuf, recvRespBufSize_, written)) {
                if (written <= 0) {
                    respMsgQueue_->rwUnlock();
                    continue;
                }

                if (recvBuf) {
                    free(recvBuf);
                    recvBuf = nullptr;
                    recvRespBufSize_ = 0;
                }

                recvBuf = malloc((size_t)written);
                if (!recvBuf) {
                    respMsgQueue_->rwUnlock();
                    veigar::log("Veigar: Error: Buffer size too small and reallocate %" PRId64 " bytes failed.\n", written);
                    continue;
                }

                recvRespBufSize_ = (uint32_t)written;
                if (!respMsgQueue_->popFront(recvBuf, recvRespBufSize_, written)) {
                    respMsgQueue_->rwUnlock();
                    continue;
                }
            }

            respMsgQueue_->rwUnlock();

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

    Veigar* veigar_ = nullptr;
    std::atomic_bool quit_ = false;
    bool isInit_ = false;
    uint32_t msgQueueCapacity_ = 0;
    uint32_t expectedMsgMaxSize_ = 0;
    uint32_t recvCallBufSize_ = 0;
    uint32_t recvRespBufSize_ = 0;

    std::atomic<uint32_t> rwTimeout_ = 260;  // ms

    std::atomic<uint32_t> callIndex_ = 0;
    std::string channelName_;
    std::string uuid_;

    // Each Veigar instance has a call queue and a response queue.
    // If A need call B's function, A push the message to B's call message queue.
    // If B has a response to A, B push the message to A's response message queue.
    //
    std::shared_ptr<MessageQueue> callMsgQueue_;
    std::shared_ptr<MessageQueue> respMsgQueue_;

    std::shared_future<void> handleCallThread_;
    std::shared_future<void> handleRespThread_;
    veigar_msgpack::unpacker callPac_;
    veigar_msgpack::unpacker respPac_;

    std::shared_ptr<RespDispatcher> respDispatcher_;
    std::shared_ptr<Sender> sender_;
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

    if (callDisp_) {
        assert(!callDisp_->isInit());
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

uint32_t Veigar::msgQueueCapacity() const {
    assert(impl_);
    return impl_->msgQueueCapacity_;
}

uint32_t Veigar::expectedMsgMaxSize() const {
    assert(impl_);
    return impl_->expectedMsgMaxSize_;
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

void Veigar::setTimeoutOfRWLock(uint32_t ms) {
    assert(impl_);
    if (ms > 0) {
        impl_->rwTimeout_.store(ms);
    }
}

uint32_t Veigar::timeoutOfRWLock() const {
    assert(impl_);
    return impl_->rwTimeout_.load();
}

std::string Veigar::getNextCallId(const std::string& funcName) const {
    assert(impl_);
    uint32_t idx = impl_->callIndex_.fetch_add(1);
    return (impl_->uuid_ + "_" + funcName + "_" + std::to_string(idx));
}

bool Veigar::sendCall(const std::string& channelName,
                      uint32_t timeoutMS,
                      std::shared_ptr<veigar_msgpack::sbuffer> buffer,
                      const std::string& callId,
                      const std::string& funcName,
                      const ResultMeta& retMeta,
                      std::string& exceptionMsg) {
    assert(impl_);
    if (!impl_->sender_) {
        return false;
    }

    Sender::CallMeta cm;
    cm.channel = channelName;
    cm.callId = callId;
    cm.resultMeta = retMeta;
    cm.dataSize = buffer->size();
    cm.data = (uint8_t*)malloc(buffer->size());
    if (!cm.data) {
        exceptionMsg = "Unable to allocate memory.";
        return false;
    }
    memcpy(cm.data, buffer->data(), buffer->size());

    cm.timeout = timeoutMS * 1000;
    cm.startCallTimePoint = std::chrono::high_resolution_clock::now();

    impl_->sender_->addCall(cm);

    return true;
}

bool Veigar::sendResponse(const std::string& targetChannel,
                          const uint8_t* buf,
                          size_t bufSize,
                          std::string& errMsg) {
    assert(impl_);
    if (!impl_->sender_) {
        return false;
    }

    Sender::RespMeta rm;
    rm.channel = targetChannel;
    rm.dataSize = bufSize;
    rm.data = (uint8_t*)malloc(bufSize);
    if (!rm.data) {
        errMsg = "Unable to allocate memory.";
        return false;
    }
    memcpy(rm.data, buf, bufSize);

    rm.timeout = VEIGAR_WRITE_RESPONSE_QUEUE_TIMEOUT * 1000;
    rm.startCallTimePoint = std::chrono::high_resolution_clock::now();

    impl_->sender_->addResp(rm);

    return true;
}

void Veigar::releaseCall(const std::string& callId) {
    if (impl_->respDispatcher_) {
        impl_->respDispatcher_->releaseCall(callId);
    }
}

}  // namespace veigar

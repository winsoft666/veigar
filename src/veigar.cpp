/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "veigar/veigar.h"
#include <cstdlib>
#include "uuid.h"
#include "log.h"
#include "message_queue.h"
#include "time_util.h"
#include "resp_dispatcher.h"
#include "sender.h"
#include "time_util.h"
#include "run_time_recorder.h"

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

            assert(sender_);
            if (!sender_->init(respDispatcher_, veigar_->callDisp_->messageQueue(), respDispatcher_->messageQueue())) {
                veigar::log("Veigar: Error: Init sender failed.\n");
                break;
            }

            isInit_ = true;
        } while (false);

        if (!isInit_) {
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

    Veigar* veigar_ = nullptr;
    bool isInit_ = false;
    uint32_t msgQueueCapacity_ = 0;
    uint32_t expectedMsgMaxSize_ = 0;

    std::atomic<uint32_t> rwTimeout_ = 260;  // ms

    std::atomic<uint32_t> callIndex_ = 0;
    std::string channelName_;
    std::string uuid_;

    // Each Veigar instance has a call queue and a response queue.
    // If A need call B's function, A push the message to B's call message queue.
    // If B has a response to A, B push the message to A's response message queue.
    //
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
    std::string callId = impl_->uuid_ + funcName + std::to_string((unsigned long)idx);
    return callId;
}

bool Veigar::sendCall(const std::string& channelName,
                      uint32_t timeoutMS,
                      std::shared_ptr<veigar_msgpack::sbuffer> buffer,
                      const std::string& callId,
                      const std::string& funcName,
                      const ResultMeta& retMeta,
                      std::string& exceptionMsg) {
    RUN_TIME_RECORDER("sendCall " + callId);
    assert(impl_);
    if (!impl_->sender_) {
        return false;
    }

    Sender::CallMeta cm;
    cm.channel = channelName;
    cm.callId = callId;
    cm.resultMeta = retMeta;
    cm.dataSize = buffer ? buffer->size() : 0;
    cm.data = (uint8_t*)malloc(cm.dataSize);
    if (!cm.data) {
        exceptionMsg = "Unable to allocate memory.";
        return false;
    }

    if (buffer) {
        memcpy(cm.data, buffer->data(), cm.dataSize);
    }

    cm.timeout = timeoutMS * 1000;
    cm.startCallTimePoint = TimeUtil::GetCurrentTimestamp();

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
    rm.startCallTimePoint = TimeUtil::GetCurrentTimestamp();

    impl_->sender_->addResp(rm);

    return true;
}

void Veigar::releaseCall(const std::string& callId) {
    if (impl_->respDispatcher_) {
        impl_->respDispatcher_->releaseCall(callId);
    }
}

}  // namespace veigar

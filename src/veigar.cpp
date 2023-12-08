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
#include "veigar/veigar.h"
#include "uuid.h"
#include "log.h"
#include "message_queue.h"
#include "time_util.h"

namespace veigar {
using CallPromise = std::pair<std::string /*caller channel name*/, std::shared_ptr<std::promise<CallResult>>>;

class Veigar::Impl {
   public:
    Impl(Veigar* parent) :
        parent_(parent) {
    }

    bool init(const std::string& channelName) {
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

            uuid_ = UUID::Create();
            if (uuid_.empty()) {
                veigar::log("Veigar: Error: Generate uuid failed.\n");
                break;
            }

            assert(parent_->disp_);
            if (!parent_->disp_->init()) {
                veigar::log("Veigar: Error: Init dispatcher failed.\n");
                break;
            }

            msgQueue_ = std::make_shared<MessageQueue>(true, VEIGAR_MAX_MESSAGE_NUMBER, VEIGAR_MAX_MESSAGE_EXPECTED_SIZE);
            if (!msgQueue_->create(channelName_)) {
                veigar::log("Veigar: Error: Create message queue(%s) failed.\n", channelName_.c_str());
                break;
            }

            recvBufSize_ = VEIGAR_MAX_MESSAGE_EXPECTED_SIZE;

            try {
                pac_.reserve_buffer(recvBufSize_);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: Error: Pre-alloc memory(%d bytes) failed: %s.\n", recvBufSize_, e.what());
                break;
            }

            try {
                recvThread_ = std::async(std::launch::async, &Impl::RecvThreadProc, this);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during starting receive thread: %s.\n", e.what());
                break;
            }
            isInit_ = true;
        } while (false);

        if (!isInit_) {
            quit_.store(true);

            if (msgQueue_) {
                msgQueue_->notifyRead();
            }

            if (recvThread_.valid()) {
                recvThread_.wait();
            }

            if (msgQueue_) {
                msgQueue_->close();
                msgQueue_.reset();
            }

            if (parent_->disp_->isInit()) {
                parent_->disp_->uninit();
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

        if (msgQueue_) {
            msgQueue_->notifyRead();
        }

        if (recvThread_.valid()) {
            recvThread_.wait();
        }

        if (msgQueue_) {
            msgQueue_->close();
            msgQueue_.reset();
        }

        ongoingCallsMutex_.lock();
        ongoingCalls_.clear();
        ongoingCallsMutex_.unlock();

        targetMQsMutex_.lock();
        for (auto it : targetMsgQueues_) {
            if (it.second) {
                it.second->close();
            }
        }
        targetMsgQueues_.clear();
        targetMQsMutex_.unlock();

        assert(parent_ && parent_->disp_);
        if (parent_->disp_->isInit()) {
            parent_->disp_->uninit();
        }

        isInit_ = false;
    }

    bool getCallPromise(const std::string& callId, CallPromise& cp) {
        std::lock_guard<std::mutex> lg(ongoingCallsMutex_);
        if (ongoingCalls_.find(callId) != ongoingCalls_.end()) {
            cp = ongoingCalls_[callId];
            return true;
        }
        return false;
    }

    std::shared_ptr<MessageQueue> getTargetMessageQueue(const std::string& channelName) {
        std::lock_guard<std::mutex> lg(targetMQsMutex_);
        std::shared_ptr<MessageQueue> queue = nullptr;
        auto it = targetMsgQueues_.find(channelName);
        if (it != targetMsgQueues_.cend()) {
            return it->second;
        }

        queue = std::make_shared<MessageQueue>(true, VEIGAR_MAX_MESSAGE_NUMBER, VEIGAR_MAX_MESSAGE_EXPECTED_SIZE);
        if (!queue->open(channelName)) {
            queue.reset();
            veigar::log("Veigar: Error: Open message queue(%s) failed.\n", channelName.c_str());
            return nullptr;
        }

        targetMsgQueues_[channelName] = queue;
        return queue;
    }

    bool sendMessage(const std::string& channelName,
                     const uint8_t* buf,
                     size_t bufSize,
                     std::string& errMsg) noexcept {
        try {
            std::shared_ptr<MessageQueue> mq = nullptr;
            if (channelName == channelName_) {
                mq = msgQueue_;
            }
            else {
                mq = getTargetMessageQueue(channelName);
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

    void RecvThreadProc() {
        void* recvBuf = malloc(recvBufSize_);
        if (!recvBuf) {
            veigar::log("Veigar: Error: Allocate receive buffer(%d bytes) failed.\n", recvBufSize_);
            return;
        }

        int64_t written = 0L;
        while (!quit_.load()) {
            written = 0L;
            if (!msgQueue_->wait(-1)) {
                continue;
            }

            if (quit_.load()) {
                break;
            }

            if (!msgQueue_->popFront(rwTimeout_.load(), recvBuf, recvBufSize_, written)) {
                if (written <= 0) {
                    continue;
                }

                if (recvBuf) {
                    free(recvBuf);
                    recvBuf = nullptr;
                    recvBufSize_ = 0;
                }

                recvBuf = malloc((size_t)written);
                if (!recvBuf) {
                    veigar::log("Veigar: Error: Buffer size too small and reallocate %" PRId64 " bytes failed.\n", written);
                    continue;
                }

                recvBufSize_ = (uint32_t)written;
                if (!msgQueue_->popFront(rwTimeout_.load(), recvBuf, recvBufSize_, written)) {
                    continue;
                }
            }

            handleMessage(recvBuf, written);
        }
    }

    void handleMessage(void* msg, int64_t msgSize) {
        assert(pac_.buffer());
        pac_.reserve_buffer((size_t)msgSize);
        memcpy(pac_.buffer(), msg, (size_t)msgSize);
        pac_.buffer_consumed((size_t)msgSize);

        do {
            std::shared_ptr<veigar_msgpack::object_handle> result = std::make_shared<veigar_msgpack::object_handle>();

            bool nextRet = false;
            try {
                veigar_msgpack::object_handle& objRef = *result;
                nextRet = pac_.next(objRef);
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

            // The first two type of CallMsg and ResponseMsg are the same.
            // std:: tuple<int8_ t. std:: string,...>;
            using CommonMsg = std::tuple<int8_t, std::string>;

            int8_t msgFlag = 99;
            std::string callId;
            CommonMsg commonMsg;

            try {
                result->get().convert(commonMsg);

                msgFlag = std::get<0>(commonMsg);
                callId = std::get<1>(commonMsg);
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during parsing received data: %s.\n", e.what());
                continue;
            } catch (...) {
                veigar::log(
                    "Veigar: Error: An exception occurred during parsing received data. The exception is not derived from std::exception. "
                    "No further information available.\n");
                continue;
            }

            if (callId.empty()) {
                veigar::log("Veigar: Warning: Call Id is empty, message flag: %d.\n", msgFlag);
                continue;
            }

            if (msgFlag == 0) {  // recv call
                assert(parent_ && parent_->disp_);
                if (parent_->disp_) {
                    parent_->disp_->pushCall(result);
                }
            }
            else if (msgFlag == 1) {  // recv response
                CallPromise callPromise;
                if (!getCallPromise(callId, callPromise)) {
                    veigar::log("Veigar: Warning: Can not find on going call (%s).\n", callId.c_str());
                    continue;
                }

                std::shared_ptr<std::promise<CallResult>> p = std::get<1>(callPromise);
                assert(p);

                if (p) {
                    CallResult callRet;
                    detail::Response resp;
                    std::string exceptionMsg;

                    if (!detail::Response::MakeResponseWithMsgpackObject(std::move(*result), resp, exceptionMsg)) {
                        callRet.errCode = ErrorCode::FAILED;
                        callRet.errorMessage = exceptionMsg;
                        p->set_value(std::move(callRet));

                        continue;
                    }

                    if (resp.getError()) {
                        callRet.errCode = ErrorCode::FAILED;
                        callRet.errorMessage = resp.getError()->get().as<std::string>();
                    }
                    else {
                        callRet.errCode = ErrorCode::SUCCESS;
                        callRet.errorMessage.clear();
                        callRet.obj = std::move(*resp.getResult());
                    }

                    p->set_value(std::move(callRet));
                }
            }
        } while (true);
    }

    Veigar* parent_ = nullptr;
    std::atomic_bool quit_ = {false};
    bool isInit_ = false;
    uint32_t recvBufSize_ = 0;

    std::atomic<uint32_t> rwTimeout_ = {260};  // ms

    std::atomic<uint32_t> callIndex_ = {0};
    std::string channelName_;
    std::string uuid_;
    std::mutex ongoingCallsMutex_;
    std::unordered_map<std::string, CallPromise> ongoingCalls_;  // callId -> CallPromise

    std::shared_ptr<MessageQueue> msgQueue_;

    std::mutex targetMQsMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> targetMsgQueues_;

    std::shared_future<void> recvThread_;
    veigar_msgpack::unpacker pac_;
};

Veigar::Veigar() noexcept :
    impl_(new Veigar::Impl(this)) {
    disp_ = std::make_shared<detail::Dispatcher>(this);
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

Veigar::~Veigar() {
    if (impl_ && impl_->recvThread_.valid()) {
        impl_->recvThread_.wait();
    }

    if (disp_) {
        disp_.reset();
    }
}

bool Veigar::init(const std::string& channelName) noexcept {
    assert(impl_);
    return impl_->init(channelName);
}

bool Veigar::isInit() const noexcept {
    assert(impl_);
    return impl_->isInit_;
}

void Veigar::uninit() noexcept {
    assert(impl_);
    impl_->uninit();
}

std::string Veigar::channelName() const noexcept {
    assert(impl_);
    return impl_->channelName_;
}

void Veigar::unbind(const std::string& funcName) noexcept {
    if (disp_) {
        disp_->unbind(funcName);
    }
}

std::vector<std::string> Veigar::bindNames() const noexcept {
    if (!disp_) {
        return {};
    }
    return disp_->names();
}

void Veigar::waitAllResponse() noexcept {
    assert(impl_);
    if (!impl_) {
        return;
    }

    try {
        for (auto& c : impl_->ongoingCalls_) {
            std::shared_ptr<std::promise<CallResult>> p = c.second.second;
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

bool Veigar::sendMessage(const std::string& targetChannel,
                         const uint8_t* buf,
                         size_t bufSize,
                         std::string& errMsg) noexcept {
    assert(impl_);
    return impl_->sendMessage(targetChannel, buf, bufSize, errMsg);
}

void Veigar::setReadWriteTimeout(uint32_t ms) noexcept {
    assert(impl_);
    if (ms > 0) {
        impl_->rwTimeout_.store(ms);
    }
}

uint32_t Veigar::readWriteTimeout() const noexcept {
    assert(impl_);
    return impl_->rwTimeout_.load();
}

std::string Veigar::getNextCallId(const std::string& funcName) const noexcept {
    assert(impl_);
    uint32_t idx = impl_->callIndex_.fetch_add(1);
    return (impl_->uuid_ + "_" + funcName + "_" + std::to_string(idx));
}

bool Veigar::sendCall(const std::string& channelName,
                      std::shared_ptr<veigar_msgpack::sbuffer> buffer,
                      const std::string& callId,
                      const std::string& funcName,
                      std::shared_ptr<std::promise<CallResult>> p,
                      std::string& exceptionMsg) noexcept {
    assert(impl_);
    do {
        std::lock_guard<std::mutex> lg(impl_->ongoingCallsMutex_);
        impl_->ongoingCalls_.insert(std::make_pair(callId, std::make_pair(funcName, p)));
    } while (false);

    return impl_->sendMessage(channelName,
                              (const uint8_t*)buffer->data(),
                              buffer->size(),
                              exceptionMsg);
}

void Veigar::releaseCall(const std::string& callId) {
    std::lock_guard<std::mutex> lg(impl_->ongoingCallsMutex_);
    auto it = impl_->ongoingCalls_.find(callId);
    if (it != impl_->ongoingCalls_.cend()) {
        impl_->ongoingCalls_.erase(it);
    }
}

}  // namespace veigar

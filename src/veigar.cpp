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
#include "shared_memory.h"
#include "event.h"

namespace veigar {

const uint32_t kPreAllocMemorySize = (1024 << 10);

using CallPromise = std::pair<std::string /*caller channel name*/, std::shared_ptr<std::promise<CallResult>>>;

class Veigar::Impl {
   public:
    Impl(Veigar* parent) :
        parent_(parent) {
    }

    static std::string GetChannelDataSegmentName(const std::string& channelName) {
        return channelName + "_DATA";
    }

    static std::string GetMessageQueueSemaphoreName(const std::string& channelName) {
        return channelName + "_SMH_543AF6DB8E074C379771FF8C03D1C0E1";
    }

    std::shared_ptr<itp::named_semaphore> getOrCreateMsgQueueSemaphore(const std::string& channelName, bool create) {
        assert(!channelName.empty());
        std::string semName;
        try {
            semName = GetMessageQueueSemaphoreName(channelName);
            std::shared_ptr<itp::named_semaphore> smh;

            if (!create) {
                smh = std::make_shared<itp::named_semaphore>(
                    itp::open_only,
                    semName.c_str());
            }
            else {
                itp::named_semaphore::remove(semName.c_str());
                smh = std::make_shared<itp::named_semaphore>(
                    itp::create_only,
                    semName.c_str(),
                    0);
            }
            return smh;
        } catch (std::exception& e) {
            veigar::log("Veigar: create semaphore(%s) exception: %s.\n", semName.c_str(), e.what());
            return nullptr;
        }
    }

    void removeMsgQueueSemaphore() {
        assert(!channelName_.empty());
        std::string semName;
        try {
            semName = GetMessageQueueSemaphoreName(channelName_);
            if (msgQueueSemaphore_) {
                msgQueueSemaphore_->remove(semName.c_str());
                msgQueueSemaphore_.reset();
            }
        } catch (std::exception& e) {
            veigar::log("Veigar: remove semaphore(%s) exception: %s.\n", semName.c_str(), e.what());
            msgQueueSemaphore_.reset();
        }
    }

    bool init(const std::string& channelName, unsigned int defaultBufferSize) {
        if (isInit_) {
            veigar::log("Veigar: already init.\n");
            return false;
        }
        do {
            quit_.store(false);

            if (channelName.empty()) {
                veigar::log("Veigar: channel name is empty.\n");
                break;
            }

            channelName_ = channelName;
            msgQueueSemaphore_ = getOrCreateMsgQueueSemaphore(channelName_, true);
            if (!msgQueueSemaphore_) {
                veigar::log("Veigar: create the semaphore of message queue failed.\n");
                break;
            }

            uuid_ = UUID::Create();
            if (uuid_.empty()) {
                veigar::log("Veigar: generate uuid failed.\n");
                break;
            }

            parent_->disp_ = std::make_shared<detail::Dispatcher>(parent_);
            if (!parent_->disp_->init()) {
                veigar::log("Veigar: init dispatcher failed.\n");
                break;
            }

            msgQueue_ = std::make_shared<MessageQueue>();

            std::string recvQueueName = GetChannelDataSegmentName(channelName_);
            if (!msgQueue_->init(recvQueueName, true, defaultBufferSize)) {
                veigar::log("Veigar: init message queue(%s) failed.\n", recvQueueName.c_str());
                break;
            }

            try {
                pac_.reserve_buffer(kPreAllocMemorySize);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: pre-alloc memory(%d bytes) failed: %s.\n", kPreAllocMemorySize, e.what());
                break;
            }

            try {
                recv_thread_ = std::async(std::launch::async, &Impl::RecvThreadProc, this);
            } catch (std::exception& e) {
                veigar::log("Veigar: start receive thread exception: %s.\n", e.what());
                break;
            }
            isInit_ = true;
        } while (false);

        if (!isInit_) {
            quit_.store(true);

            if (msgQueueSemaphore_) {
                msgQueueSemaphore_->post();
            }

            if (msgQueue_) {
                if (msgQueue_->isInit()) {
                    msgQueue_->uninit();
                }
            }

            if (recv_thread_.valid()) {
                recv_thread_.wait();
            }

            removeMsgQueueSemaphore();

            if (msgQueue_) {
                msgQueue_.reset();
            }

            if (parent_->disp_) {
                if (parent_->disp_->isInit()) {
                    parent_->disp_->uninit();
                }
                parent_->disp_.reset();
            }

            uuid_.clear();
            channelName_.clear();
        }
        else {
            veigar::log("Veigar: init success, channel: %s, uuid: %s.\n", channelName_.c_str(), uuid_.c_str());
        }

        return isInit_;
    }

    void uninit() {
        if (isInit_) {
            quit_.store(true);

            assert(msgQueueSemaphore_);
            if (msgQueueSemaphore_) {
                msgQueueSemaphore_->post();
            }

            assert(msgQueue_);
            if (msgQueue_) {
                if (msgQueue_->isInit()) {
                    msgQueue_->uninit();
                }
            }

            if (recv_thread_.valid()) {
                recv_thread_.wait();
            }

            if (msgQueue_) {
                msgQueue_.reset();
            }

            removeMsgQueueSemaphore();

            if (parent_->disp_) {
                if (parent_->disp_->isInit()) {
                    parent_->disp_->uninit();
                }
                parent_->disp_.reset();
            }

            isInit_ = false;
        }
    }

    bool getCallPromise(const std::string& callId, CallPromise& cp) {
        std::lock_guard<std::mutex> lg(ongoingCallsMutex_);
        if (ongoingCalls_.find(callId) != ongoingCalls_.end()) {
            cp = ongoingCalls_[callId];
            return true;
        }
        return false;
    }

    std::shared_ptr<MessageQueue> getMessageQueue(const std::string& channelName) {
        std::lock_guard<std::mutex> lg(channelMsgQueueMutex_);
        std::shared_ptr<MessageQueue> queue = nullptr;
        if (channelMsgQueue_.find(channelName) != channelMsgQueue_.end()) {
            return channelMsgQueue_[channelName];
        }

        queue = std::make_shared<MessageQueue>();
        if (!queue->init(GetChannelDataSegmentName(channelName), false, 0)) {
            return nullptr;
        }

        channelMsgQueue_[channelName] = queue;
        return queue;
    }

    std::shared_ptr<itp::named_semaphore> getMessageQueueShm(const std::string& channelName) {
        std::lock_guard<std::mutex> lg(channelMsgQueueSmhMutex_);
        if (channelMsgQueueSmh_.find(channelName) != channelMsgQueueSmh_.end()) {
            return channelMsgQueueSmh_[channelName];
        }

        std::shared_ptr<itp::named_semaphore> smh = getOrCreateMsgQueueSemaphore(channelName, false);
        if (!smh) {
            return nullptr;
        }

        channelMsgQueueSmh_[channelName] = smh;
        return smh;
    }

    bool sendMessage(const std::string& channelName,
                     const uint8_t* buf,
                     size_t bufSize,
                     unsigned int timeout,
                     std::string& errMsg) noexcept {
        try {
            std::shared_ptr<MessageQueue> mq = nullptr;
            std::shared_ptr<itp::named_semaphore> shm = nullptr;
            if (channelName == channelName_) {
                mq = msgQueue_;
            }
            else {
                mq = getMessageQueue(channelName);
            }
            if (!mq) {
                errMsg = "Unable to get target message queue. It seems that the channel not started.";
                return false;
            }

            if (channelName == channelName_) {
                shm = msgQueueSemaphore_;
            }
            else {
                shm = getMessageQueueShm(channelName);
            }

            if (!shm) {
                errMsg = "Unable to get target semaphore. It seems that the channel not started.";
                return false;
            }

            std::vector<uint8_t> data;
            data.resize(bufSize);
            memcpy(&data[0], buf, bufSize);

            if (!mq->pushBack(data, timeout)) {
                return false;
            }

            if (shm) {
                shm->post();
            }

            return true;
        } catch (std::exception& e) {
            errMsg = e.what();
            return false;
        }
    }

    void RecvThreadProc() {
        assert(msgQueueSemaphore_);
        if (!msgQueueSemaphore_)
            return;

        while (!quit_.load()) {
            bool got = false;
            try {
                if (msgQueueSemaphore_) {
                    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
                    got = msgQueueSemaphore_->timed_wait(deadline);
                }
            } catch (std::exception& e) {
                veigar::log("Veigar: wait message queue semaphore exception: %s.\n", e.what());
            }

            if (!got) {
                continue;
            }

            try {
                Message msg = msgQueue_->createMessage();
                if (!msgQueue_->popFront(&msg, 100)) {  // 100ms lock timeout
                    break;
                }

                if (msg.isEmpty()) {
                    continue;
                }

                std::vector<uint8_t> recvBuf = msg.getArg();
                if (recvBuf.size() > 0) {
                    handleMessage(recvBuf);
                }
                else {
                    veigar::log("Veigar: the size of received message is zero.\n");
                }
            } catch (std::exception& e) {
                veigar::log("Veigar: exception on message receive thread process: %s.\n", e.what());
                assert(false);
            }
        }
    }

    void handleMessage(const std::vector<uint8_t>& recvBuf) {
        assert(pac_.buffer());
        pac_.reserve_buffer(recvBuf.size());
        memcpy(pac_.buffer(), recvBuf.data(), recvBuf.size());
        pac_.buffer_consumed(recvBuf.size());

        do {
            std::shared_ptr<veigar_msgpack::object_handle> result = std::make_shared<veigar_msgpack::object_handle>();

            bool nextRet = false;
            try {
                veigar_msgpack::object_handle& objRef = *result;
                nextRet = pac_.next(objRef);
            } catch (std::exception& e) {
                veigar::log("Veigar: parse received data exception: %s.\n", e.what());
                nextRet = false;
            } catch (...) {
                veigar::log(
                    "Veigar: parse received data exception. The exception is not derived from std::exception. "
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
                veigar::log("Veigar: parse received data exception: %s.\n", e.what());
                continue;
            } catch (...) {
                veigar::log(
                    "Veigar: parse received data exception. The exception is not derived from std::exception. "
                    "No further information available.\n");
                continue;
            }

            if (callId.empty()) {
                veigar::log("Veigar: callId is empty, msgFlag: %d.\n", msgFlag);
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
                    veigar::log(
                        "Veigar: can not find on going call (callId: %s), "
                        "possible the call invoked by other channel.\n",
                        callId.c_str());
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
    std::shared_ptr<itp::named_semaphore> msgQueueSemaphore_;
    std::atomic_bool quit_ = false;
    bool isInit_ = false;

    std::atomic<uint32_t> callIndex_ = 0;
    std::string channelName_;
    std::string uuid_;
    std::mutex ongoingCallsMutex_;
    std::unordered_map<std::string, CallPromise> ongoingCalls_;  // callId -> CallPromise

    std::shared_ptr<MessageQueue> msgQueue_;

    std::mutex channelMsgQueueMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> channelMsgQueue_;

    std::mutex channelMsgQueueSmhMutex_;
    std::unordered_map<std::string, std::shared_ptr<itp::named_semaphore>> channelMsgQueueSmh_;

    std::shared_future<void> recv_thread_;
    veigar_msgpack::unpacker pac_;
};

Veigar::Veigar() noexcept :
    impl_(new Veigar::Impl(this)) {
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
    if (impl_ && impl_->recv_thread_.valid()) {
        impl_->recv_thread_.wait();
    }
}

bool Veigar::init(const std::string& channelName, unsigned int defaultBufferSize) noexcept {
    assert(impl_);
    return impl_->init(channelName, defaultBufferSize);
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
        veigar::log("Veigar: wait all response exception: %s\n", e.what());
    }
}

bool Veigar::sendMessage(const std::string& targetChannel,
                         const uint8_t* buf,
                         size_t bufSize,
                         unsigned int timeoutMS,
                         std::string& errMsg) noexcept {
    return impl_->sendMessage(targetChannel, buf, bufSize, timeoutMS, errMsg);
}

std::string Veigar::getNextCallId(const std::string& funcName) const noexcept {
    uint32_t idx = impl_->callIndex_.fetch_add(1);
    return impl_->uuid_ + "_" + funcName + "_" + std::to_string(idx);
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
                              100,
                              exceptionMsg);
}

void Veigar::releaseCall(const std::string& callId) {
    std::lock_guard<std::mutex> lg(impl_->ongoingCallsMutex_);
    if (impl_->ongoingCalls_.find(callId) != impl_->ongoingCalls_.cend()) {
        impl_->ongoingCalls_.erase(callId);
    }
}

}  // namespace veigar

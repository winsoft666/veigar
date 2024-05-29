/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "veigar/call_dispatcher.h"
#include "string_helper.h"
#include "log.h"
#include "veigar/veigar.h"
#include "time_util.h"
#include <atomic>
#include <queue>
#include "message_queue.h"
#include "run_time_recorder.h"

namespace veigar {
namespace detail {
using detail::Response;

class CallDispatcher::Impl {
   public:
    std::vector<std::thread> workers_;
    std::atomic_bool stop_ = false;
    std::shared_ptr<MessageQueue> callMsgQueue_;
};

CallDispatcher::CallDispatcher(Veigar* veigar) noexcept :
    veigar_(veigar),
    impl_(new Impl()) {
}

CallDispatcher::~CallDispatcher() noexcept {
    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

bool CallDispatcher::init() {
    if (init_) {
        return true;
    }

    impl_->callMsgQueue_ = std::make_shared<MessageQueue>(veigar_->msgQueueCapacity(), veigar_->expectedMsgMaxSize());
    if (!impl_->callMsgQueue_->create(veigar_->channelName() + VEIGAR_CALL_QUEUE_NAME_SUFFIX)) {
        veigar::log("Veigar: Error: Create call message queue(%s) failed.\n", veigar_->channelName().c_str());
        return false;
    }

    impl_->stop_.store(false);

    for (size_t i = 0; i < VEIGAR_DISPATCHER_THREAD_NUMBER; ++i) {
        impl_->workers_.emplace_back(std::thread(&CallDispatcher::dispatchThreadProc, this));
    }

    init_ = true;

    return init_;
}

bool CallDispatcher::isInit() const {
    return init_;
}

void CallDispatcher::uninit() {
    if (!init_) {
        return;
    }

    impl_->stop_.store(true);

    for (std::thread& worker : impl_->workers_) {
        impl_->callMsgQueue_->notifyRead();
    }

    for (std::thread& worker : impl_->workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (impl_->callMsgQueue_) {
        impl_->callMsgQueue_->close();
        impl_->callMsgQueue_.reset();
    }

    funcs_.clear();

    init_ = false;
}

std::shared_ptr<veigar::MessageQueue> CallDispatcher::messageQueue() {
    return impl_->callMsgQueue_;
}

void CallDispatcher::unbind(std::string const& name) {
    auto it = funcs_.find(name);
    if (it != funcs_.end()) {
        funcs_.erase(it);
    }
}

Response CallDispatcher::dispatch(veigar_msgpack::object const& msg, std::string& callerChannelName) {
    // Quickly check
    if (msg.via.array.size != 5) {
        return Response::MakeEmptyResponse();
    }

    return dispatchCall(msg, callerChannelName);
}

Response CallDispatcher::dispatchCall(veigar_msgpack::object const& msg, std::string& callerChannelName) {
    CallMsg the_call;
    try {
        msg.convert(the_call);
    } catch (std::exception& e) {
        veigar::log("Veigar: Error: An exception occurred during parsing response message: %s.\n", e.what());
        return Response::MakeEmptyResponse();
    } catch (...) {
        veigar::log("Veigar: Error: An exception occurred during parsing response message.\n");
        return Response::MakeEmptyResponse();
    }

    // proper validation of protocol (and responding to it)
    auto&& type = std::get<0>(the_call);
    assert(type == 0);

    auto&& callId = std::get<1>(the_call);

    if (type != 0) {
        return Response::MakeResponseWithError(callId, std::string("Invalid message flag."));
    }

    callerChannelName = std::get<2>(the_call);
    auto&& funcName = std::get<3>(the_call);
    auto&& args = std::get<4>(the_call);

    std::unordered_map<std::string, AdaptorType>::const_iterator itFunc = funcs_.find(funcName);
    if (itFunc == funcs_.cend()) {
        return Response::MakeResponseWithError(
            callId,
            StringHelper::StringPrintf("Could not find function '%s' with argument count %d.", funcName.c_str(), args.via.array.size));
    }

    try {
        auto result = (itFunc->second)(args);
        return Response::MakeResponseWithResult(callId, std::move(result));
    } catch (std::exception& e) {
        return Response::MakeResponseWithError(
            callId,
            StringHelper::StringPrintf("Function '%s' (called with %d arg(s)) threw an exception. "
                                       "The exception contained this information: %s.",
                                       funcName.c_str(), args.via.array.size, e.what()));
    } catch (...) {
        return Response::MakeResponseWithError(
            callId,
            StringHelper::StringPrintf("Function '%s' (called with %d arg(s)) threw an exception. "
                                       "The exception is not derived from std::exception. No further information available.",
                                       funcName.c_str(), args.via.array.size));
    }
}

void CallDispatcher::dispatchThreadProc() {
    veigar_msgpack::unpacker callPac;
    try {
        callPac.reserve_buffer(veigar_->expectedMsgMaxSize());
    } catch (std::bad_alloc& e) {
        veigar::log("Veigar: Error: Pre-alloc call memory(%u bytes) failed: %s.\n", veigar_->expectedMsgMaxSize(), e.what());
        return;
    }

    int64_t written = 0L;
    while (!impl_->stop_.load()) {
        written = 0L;
        if (!impl_->callMsgQueue_->wait(-1)) {
            continue;
        }

        RUN_TIME_RECORDER("callDispatchThreadProc");

        if (impl_->stop_.load()) {
            break;
        }

        if (!impl_->callMsgQueue_->rwLock(veigar_->timeoutOfRWLock())) {
            veigar::log("Veigar: Warning: Get rw-lock timeout when pop front from call message queue.\n");
            continue;
        }

        if (!impl_->callMsgQueue_->popFront(callPac.buffer(), callPac.buffer_capacity(), written)) {
            if (written <= 0) {
                veigar::log("Veigar: Error: Pop front from call message queue failed.\n");
                impl_->callMsgQueue_->rwUnlock();
                continue;
            }

            try {
                callPac.reserve_buffer((size_t)written);
            } catch (std::bad_alloc& e) {
                veigar::log("Veigar: Error: Pre-alloc call memory(%u bytes) failed: %s.\n", written, e.what());
                impl_->callMsgQueue_->rwUnlock();
                continue;
            }

            if (!impl_->callMsgQueue_->popFront(callPac.buffer(), callPac.buffer_capacity(), written)) {
                veigar::log("Veigar: Error: Pop front from call message queue failed.\n");
                impl_->callMsgQueue_->rwUnlock();
                continue;
            }
        }

        callPac.buffer_consumed((size_t)written);

        impl_->callMsgQueue_->rwUnlock();

        do {
            veigar_msgpack::object_handle obj;
            bool nextRet = false;
            try {
                nextRet = callPac.next(obj);
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

            try {
                auto msg = obj.get();

                std::string callerChannelName;
                Response resp = dispatch(msg, callerChannelName);
                if (callerChannelName.empty()) {
                    veigar::log("Veigar: Warning: Unable to parse caller's channel name.\n");
                    continue;
                }

                veigar_msgpack::sbuffer respBuf = resp.getData();
                if (respBuf.size() == 0) {
                    veigar::log("Veigar: Warning: The size of response data is zero.\n");
                    continue;
                }

                std::string errMsg;
                if (!veigar_->sendResponse(callerChannelName, (const uint8_t*)respBuf.data(), respBuf.size(), errMsg)) {
                    veigar::log("Veigar: Error: Send response to caller failed, caller: %s, error: %s.\n",
                                callerChannelName.c_str(), errMsg.c_str());
                }
            } catch (std::exception& e) {
                veigar::log("Veigar: Error: An exception occurred during handling dispatch call: %s.\n", e.what());
            }

        } while (true);
    }
}

bool CallDispatcher::isFuncNameExist(std::string const& func) {
    auto pos = funcs_.find(func);
    if (pos != end(funcs_)) {
        return true;
    }
    return false;
}

}  // namespace detail
}  // namespace veigar

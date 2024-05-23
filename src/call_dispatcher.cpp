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
#include "semaphore.h"
#include <atomic>
#include <queue>

namespace veigar {
namespace detail {
using detail::Response;

class CallDispatcher::Impl {
   public:
    std::vector<std::thread> workers_;
    std::queue<std::shared_ptr<veigar_msgpack::object_handle>> objs_;
    std::mutex objsMutex_;

    std::atomic_bool stop_ = {false};
    std::shared_ptr<Semaphore> smh_ = nullptr;
};

CallDispatcher::CallDispatcher(Veigar* parent) noexcept :
    parent_(parent),
    impl_(new Impl()) {
    impl_->smh_ = std::make_shared<Semaphore>();
}

CallDispatcher::~CallDispatcher() noexcept {
    if (impl_) {
        if (impl_->smh_) {
            if (impl_->smh_->valid()) {
                impl_->smh_->close();
            }
            impl_->smh_.reset();
        }

        delete impl_;
        impl_ = nullptr;
    }
}

bool CallDispatcher::init() {
    if (init_) {
        return true;
    }

    impl_->stop_.store(false);

    if (!impl_->smh_->open("")) {
        return false;
    }

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

    if (impl_->smh_) {
        const size_t releaseNum = impl_->workers_.size() * 2;
        for (size_t i = 0; i < releaseNum; i++) {
            impl_->smh_->release();
        }
    }

    for (std::thread& worker : impl_->workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (impl_->smh_) {
        if (impl_->smh_->valid()) {
            impl_->smh_->close();
        }
    }

    funcs_.clear();

    init_ = false;
}

void CallDispatcher::unbind(std::string const& name) {
    auto it = funcs_.find(name);
    if (it != funcs_.end()) {
        funcs_.erase(it);
    }
}

void CallDispatcher::pushCall(std::shared_ptr<veigar_msgpack::object_handle> result) {
    impl_->objsMutex_.lock();
    impl_->objs_.emplace(result);
    impl_->objsMutex_.unlock();

    impl_->smh_->release();
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
    std::string errMsg;
    while (true) {
        impl_->smh_->wait();
        if (impl_->stop_.load()) {
            break;
        }

        std::shared_ptr<veigar_msgpack::object_handle> obj = nullptr;
        do {
            std::lock_guard<std::mutex> lock(impl_->objsMutex_);
            if (!impl_->objs_.empty()) {
                obj = std::move(impl_->objs_.front());
                impl_->objs_.pop();
            }
        } while (false);

        if (!obj) {
            continue;
        }

        try {
            auto msg = obj->get();

            std::string callerChannelName;
            Response resp = dispatch(msg, callerChannelName);
            if (callerChannelName.empty()) {
                continue;
            }

            veigar_msgpack::sbuffer respBuf = resp.getData();
            if (respBuf.size() == 0) {
                veigar::log("Veigar: Warning: The size of response data is zero.\n");
                continue;
            }

            errMsg.clear();
            if (!parent_->sendMessage(callerChannelName, false, (const uint8_t*)respBuf.data(), respBuf.size(), errMsg)) {
                veigar::log("Veigar: Error: Send response to caller failed, caller: %s, error: %s.\n",
                            callerChannelName.c_str(), errMsg.c_str());
            }
        } catch (std::exception& e) {
            veigar::log("Veigar: Error: An exception occurred during handling dispatch call: %s.\n", e.what());
        }
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

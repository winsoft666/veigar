/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
namespace veigar {
template <typename F>
bool Veigar::bind(const std::string& funcName, F func) {
    if (!callDisp_) {
        return false;
    }
    return callDisp_->bind(funcName, func);
}

template <typename... Args>
std::shared_ptr<AsyncCallResult> Veigar::asyncCall(const std::string& targetChannel,
                                                   const std::string& funcName,
                                                   Args... args) {
    return doAsyncCall(targetChannel, funcName, std::forward<Args>(args)...);
}

template <typename... Args>
void Veigar::asyncCall(
    ResultCallback cb,
    const std::string& targetChannel,
    const std::string& funcName,
    Args... args) {
    doAsyncCallWithCallback(cb, targetChannel, funcName, std::forward<Args>(args)...);
}

template <typename... Args>
CallResult Veigar::syncCall(const std::string& targetChannel,
                            uint32_t timeoutMS,
                            const std::string& funcName,
                            Args... args) {
    std::shared_ptr<AsyncCallResult> acr = doAsyncCall(targetChannel, funcName, std::forward<Args>(args)...);
    if (!acr || !acr->second.valid()) {
        if (acr) {
            releaseCall(acr->first);
        }

        CallResult callRet;
        callRet.errCode = ErrorCode::FAILED;
        callRet.errorMessage = "Unknown Error.";
        return callRet;
    }

    if (timeoutMS > 0) {
        auto waitResult = acr->second.wait_for(std::chrono::milliseconds(timeoutMS));
        if (waitResult == std::future_status::timeout) {
            releaseCall(acr->first);

            CallResult callRet;
            callRet.errCode = ErrorCode::TIMEOUT;
            callRet.errorMessage = "Timeout.";
            return callRet;
        }
    }

    CallResult callRet = acr->second.get();

    releaseCall(acr->first);

    return callRet;
}

template <typename... Args>
std::shared_ptr<AsyncCallResult> Veigar::doAsyncCall(const std::string& targetChannel,
                                                     const std::string& funcName,
                                                     Args... args) {
    CallResult failedRet;
    failedRet.errCode = ErrorCode::FAILED;

    std::shared_ptr<AsyncCallResult> acr = std::make_shared<AsyncCallResult>();
    const std::string callId = getNextCallId(funcName);
    assert(!callId.empty());
    if (callId.empty()) {
        return nullptr;
    }

    acr->first = callId;
    auto p = std::make_shared<std::promise<CallResult>>();

    try {
        auto ft = p->get_future();

        std::string curChannelName = channelName();
        auto argsObj = std::make_tuple(args...);
        auto callObj = std::make_tuple(0, callId, curChannelName, funcName, argsObj);

        auto buffer = std::make_shared<veigar_msgpack::sbuffer>();
        veigar_msgpack::pack(*buffer, callObj);

        ResultMeta retMeta;
        retMeta.metaType = 0;
        retMeta.p = p;

        std::string errMsg;
        if (!sendCall(targetChannel, buffer, callId, funcName, retMeta, errMsg)) {
            if (errMsg.empty()) {
                failedRet.errorMessage = "Send failed: Unknown.";
            }
            else {
                failedRet.errorMessage = "Send failed: " + errMsg;
            }

            p->set_value(std::move(failedRet));
        }

        acr->second = std::move(ft);
    } catch (std::exception& e) {
        failedRet.errorMessage = e.what();
        p->set_value(std::move(failedRet));
    }

    return acr;
}

template <typename... Args>
void Veigar::doAsyncCallWithCallback(
    ResultCallback cb,
    const std::string& targetChannel,
    const std::string& funcName,
    Args... args) {
    CallResult failedRet;
    failedRet.errCode = ErrorCode::FAILED;

    const std::string callId = getNextCallId(funcName);
    assert(!callId.empty());
    if (callId.empty()) {
        if (cb) {
            failedRet.errorMessage = "Unable to generate call id.";
            cb(failedRet);
        }
        return;
    }

    try {
        std::string curChannelName = channelName();
        auto argsObj = std::make_tuple(args...);
        auto callObj = std::make_tuple(0, callId, curChannelName, funcName, argsObj);

        auto buffer = std::make_shared<veigar_msgpack::sbuffer>();
        veigar_msgpack::pack(*buffer, callObj);

        ResultMeta retMeta;
        retMeta.metaType = 1;
        retMeta.cb = cb;

        std::string errMsg;
        if (!sendCall(targetChannel, buffer, callId, funcName, retMeta, errMsg)) {
            if (errMsg.empty()) {
                failedRet.errorMessage = "Send failed: Unknown.";
            }
            else {
                failedRet.errorMessage = "Send failed: " + errMsg;
            }

            if (cb) {
                cb(failedRet);
            }
            return;
        }
    } catch (std::exception& e) {
        failedRet.errorMessage = e.what();
        if (cb) {
            cb(failedRet);
        }
        return;
    }
}
}  // namespace veigar
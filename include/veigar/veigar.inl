namespace veigar {
template <typename F>
bool Veigar::bind(const std::string& funcName, F func) noexcept {
    if (!disp_) {
        return false;
    }
    return disp_->bind(funcName, func);
}

template <typename... Args>
std::shared_ptr<AsyncCallResult> Veigar::asyncCall(const std::string& targetChannel, const std::string& funcName, Args... args) noexcept {
    return doAsyncCall(targetChannel, funcName, std::forward<Args>(args)...);
}

template <typename... Args>
CallResult Veigar::syncCall(const std::string& targetChannel, unsigned int timeoutMS, const std::string& funcName, Args... args) noexcept {
    std::shared_ptr<AsyncCallResult> acr = doAsyncCall(targetChannel, funcName, std::forward<Args>(args)...);
    if (!acr || !acr->second.valid()) {
        releaseCall(acr->first);

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
            callRet.errorMessage = "Timeout";
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
                                                     Args... args) noexcept {
    CallResult failedRet;
    failedRet.errCode = ErrorCode::FAILED;

    std::shared_ptr<AsyncCallResult> acr = std::make_shared<AsyncCallResult>();
    const std::string callId = getNextCallId(funcName);
    // TODO Ensure that callId is always generated
    assert(!callId.empty());

    acr->first = callId;
    auto p = std::make_shared<std::promise<CallResult>>();

    try {
        auto ft = p->get_future();

        std::string curChannelName = channelName();
        auto argsObj = std::make_tuple(args...);
        auto callObj = std::make_tuple(0, callId, curChannelName, funcName, argsObj);

        auto buffer = std::make_shared<veigar_msgpack::sbuffer>();
        veigar_msgpack::pack(*buffer, callObj);

        std::string errMsg;
        if (!sendCall(targetChannel, buffer, callId, funcName, p, errMsg)) {
            if (errMsg.empty()) {
                failedRet.errorMessage = "Unknown Error.";
            }
            else {
                failedRet.errorMessage = errMsg;
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
}  // namespace veigar
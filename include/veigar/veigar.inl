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
CallResult Veigar::syncCall(const std::string& targetChannel, uint32_t timeoutMS, const std::string& funcName, Args... args) noexcept {
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
                                                     Args... args) noexcept {
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
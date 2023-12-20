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
#ifndef VEIGAR_CALL_RESULT_H_
#define VEIGAR_CALL_RESULT_H_
#pragma once

#include <future>
#include "veigar/msgpack.hpp"

namespace veigar {
enum class ErrorCode {
    // This success only indicates that the message has been delivered to target message queue,
    // does not mean that the target successfully parse or execute the calling function.
    // If the 'errorMessage' is not empty, it indicates an error during parsing and execution by the target.
    SUCCESS = 0,
    TIMEOUT = 1,
    FAILED = 2,
};

class CallResult {
   public:
    CallResult() = default;
    CallResult(CallResult&& other) noexcept {
        *this = std::move(other);
    }

    CallResult& operator=(CallResult&& other) noexcept {
        if (this != &other) {
            errCode = std::move(other.errCode);
            errorMessage = std::move(other.errorMessage);
            obj = std::move(other.obj);
        }
        return *this;
    }

   public:
    ErrorCode errCode = ErrorCode::FAILED;
    std::string errorMessage;

    // MsgPack object, 'convertObject' function can be used to convert 'obj' to the corresponding type.
    veigar_msgpack::object_handle obj;

    bool isSuccess() const {
        return errCode == ErrorCode::SUCCESS;
    }

    template <typename T>
    bool convertObject(T& t) {
        bool result = true;
        try {
            t = obj.get().as<T>();
        } catch (...) {
            result = false;
        }
        return result;
    }

   private:
    CallResult(const CallResult&) = delete;
    CallResult& operator=(const CallResult& other) = delete;
};

using AsyncCallResult = std::pair<std::string /* call id*/, std::future<CallResult>>;
}  // namespace veigar

#endif  // !VEIGAR_CALL_RESULT_H_
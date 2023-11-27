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
#ifndef VEIGAR_RESPONSE_H_
#define VEIGAR_RESPONSE_H_
#pragma once

#include "veigar/detail/make_unique.h"
#include "veigar/msgpack.hpp"

namespace veigar {
namespace detail {
// Represents a response and creates a msgpack to be sent back as per the msgpack-rpc spec.
class Response {
   public:
    // The type of a response, according to the msgpack-rpc spec.
    // flag(1) - callId - error - result
    using ResponseMsg = std::tuple<int8_t, std::string, veigar_msgpack::object, veigar_msgpack::object>;

    // Default constructor for responses.
    Response();

    // Creates a response that represents a normal return value.
    // param id The sequence id (as per protocol).
    // param result The return value to store in the response.
    // param T Any msgpack-able type.
    //
    // If there is both an error and result in the response,
    // the result will be discarded while packing the data.
    template <typename T>
    static Response MakeResponseWithResult(const std::string& callId, T&& result);

    // Creates a response that represents an error.
    // param id The sequence id (as per protocol).
    // param error The error value to store in the response.
    // param T Any msgpack-able type.
    template <typename T>
    static Response MakeResponseWithError(const std::string& callId, T&& error);

    // Gets an empty response which means "no response" (not to be confused with void return)
    static Response MakeEmptyResponse();

    // Creates a response from veigar_msgpack::object (useful when reading a response from a stream).
    static bool MakeResponseWithMsgpackObject(veigar_msgpack::object_handle o, Response& resp, std::string& exceptionMsg);

    // Gets the response data as a veigar_msgpack::sbuffer.
    veigar_msgpack::sbuffer getData() const;

    // Moves the specified object_handle into the response as a result.
    // param r The result to capture.
    void setResult(veigar_msgpack::object_handle& r);

    // Moves the specified object_handle into the response as an error.
    // param e The error to capture.
    void setError(veigar_msgpack::object_handle& e);

    // Returns the call id used to identify which call this response corresponds to.
    std::string getCallId() const;

    // Returns the error object stored in the response. Can be empty.
    std::shared_ptr<veigar_msgpack::object_handle> getError() const;

    // Returns the result stored in the response. Can be empty.
    std::shared_ptr<veigar_msgpack::object_handle> getResult() const;

    // If true, this response is empty (see MakeEmptyResponse())
    bool isEmpty() const;

   private:
    std::string callId_;

    std::shared_ptr<veigar_msgpack::object_handle> error_;
    std::shared_ptr<veigar_msgpack::object_handle> result_;
};

template <typename T>
inline Response Response::MakeResponseWithResult(const std::string& callId, T&& result) {
    auto z = veigar::detail::make_unique<veigar_msgpack::zone>();
    veigar_msgpack::object o(std::forward<T>(result), *z);
    Response inst;
    inst.callId_ = callId;
    inst.result_ = std::make_shared<veigar_msgpack::object_handle>(o, std::move(z));
    return inst;
}

template <>
inline Response Response::MakeResponseWithResult(const std::string& callId, std::unique_ptr<veigar_msgpack::object_handle>&& r) {
    Response inst;
    inst.callId_ = callId;
    inst.result_ = std::move(r);
    return inst;
}

template <typename T>
inline Response Response::MakeResponseWithError(const std::string& callId, T&& error) {
    auto z = veigar::detail::make_unique<veigar_msgpack::zone>();
    veigar_msgpack::object o(std::forward<T>(error), *z);
    Response inst;
    inst.callId_ = callId;
    inst.error_ = std::make_shared<veigar_msgpack::object_handle>(o, std::move(z));
    return inst;
}
}  // namespace detail
}  // namespace veigar
#endif  // !VEIGAR_RESPONSE_H_

#ifndef VEIGAR_RESPONSE_H_
#define VEIGAR_RESPONSE_H_
#pragma once

#include "veigar/detail/make_unique.h"
#include "veigar/msgpack.hpp"

namespace veigar {
namespace detail {
//! \brief Represents a response and creates a msgpack to be sent back as per the msgpack-rpc spec.
class Response {
   public:
    //! \brief The type of a response, according to the msgpack-rpc spec.
    //! flag(1) - callId - error - result
    using ResponseMsg = std::tuple<int8_t, std::string, veigar_msgpack::object, veigar_msgpack::object>;

    //! \brief Default constructor for responses.
    Response();

    //! \brief Creates a response that represents a normal return value.
    //! \param id The sequence id (as per protocol).
    //! \param result The return value to store in the response.
    //! \tparam T Any msgpack-able type.
    //! \note If there is both an error and result in the response,
    //! the result will be discarded while packing the data.
    template <typename T>
    static Response MakeResponseWithResult(const std::string& callId, T&& result);

    //! \brief Creates a response that represents an error.
    //! \param id The sequence id (as per protocol).
    //! \param error The error value to store in the response.
    //! \tparam T Any msgpack-able type.
    template <typename T>
    static Response MakeResponseWithError(const std::string& callId, T&& error);

    //! \brief Gets an empty response which means "no response" (not to be
    //! confused with void return, i.e. this means literally "don't write the response to the socket")
    static Response MakeEmptyResponse();

    //! \brief Creates a response from veigar_msgpack::object (useful when reading a response from a stream).
    static bool MakeResponseWithMsgpackObject(veigar_msgpack::object_handle o, Response& resp, std::string& exceptionMsg);

    //! \brief Gets the response data as a veigar_msgpack::sbuffer.
    veigar_msgpack::sbuffer getData() const;

    //! \brief Moves the specified object_handle into the response as a result.
    //! \param r The result to capture.
    void setResult(veigar_msgpack::object_handle& r);

    //! \brief Moves the specified object_handle into the response as an error.
    //! \param e The error to capture.
    void setError(veigar_msgpack::object_handle& e);

    //! \brief Returns the call id used to identify which call this response corresponds to.
    std::string getCallId() const;

    //! \brief Returns the error object stored in the response. Can be empty.
    std::shared_ptr<veigar_msgpack::object_handle> getError() const;

    //! \brief Returns the result stored in the response. Can be empty.
    std::shared_ptr<veigar_msgpack::object_handle> getResult() const;

    //! \brief If true, this response is empty (\see MakeEmptyResponse())
    bool isEmpty() const;

   private:
    std::string callId_;

    // I really wish to avoid shared_ptr here but at this point asio does not
    // work with move-only handlers in post() and I need to capture responses
    // in lambdas.
    std::shared_ptr<veigar_msgpack::object_handle> error_;
    std::shared_ptr<veigar_msgpack::object_handle> result_;
};

template <typename T>
inline Response Response::MakeResponseWithResult(const std::string& callId, T&& result) {
    auto z = rpc::detail::make_unique<veigar_msgpack::zone>();
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

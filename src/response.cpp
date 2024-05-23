/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "veigar/config.h"
#include "veigar/detail/response.h"
#include <assert.h>
#include "log.h"
#include "string_helper.h"

namespace veigar {
namespace detail {

veigar_msgpack::sbuffer Response::getData() const {
    veigar_msgpack::sbuffer data;
    ResponseMsg r(1,
                  callId_,
                  error_ ? error_->get() : veigar_msgpack::object(),
                  result_ ? result_->get() : veigar_msgpack::object());
    veigar_msgpack::pack(data, r);
    return data;
}

std::string Response::getCallId() const {
    return callId_;
}

std::shared_ptr<veigar_msgpack::object_handle> Response::getError() const {
    return error_;
}

std::shared_ptr<veigar_msgpack::object_handle> Response::getResult() const {
    return result_;
}

Response Response::MakeEmptyResponse() {
    Response r;
    return r;
}

bool Response::isEmpty() const {
    return (!error_ && !result_);
}

void Response::setResult(veigar_msgpack::object_handle& r) {
    if (!result_) {
        result_ = std::make_shared<veigar_msgpack::object_handle>();
    }
    result_->set(std::move(r).get());
}

void Response::setError(veigar_msgpack::object_handle& e) {
    if (!error_) {
        error_ = std::shared_ptr<veigar_msgpack::object_handle>();
    }
    error_->set(std::move(e).get());
}

}  // namespace detail
}  // namespace veigar

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
#include "veigar/config.h"
#include "veigar/detail/response.h"
#include <assert.h>
#include "log.h"
#include "string_helper.h"

namespace veigar {
namespace detail {

Response::Response() :
    callId_(),
    error_(),
    result_() {}

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

bool Response::MakeResponseWithMsgpackObject(veigar_msgpack::object_handle o, Response& resp, std::string& exceptionMsg) {
    try {
        ResponseMsg r;
        o.get().convert(r);

        // check protocol
        uint32_t msgFlag = std::get<0>(r);
        if (msgFlag != 1) {
            exceptionMsg = "Invalid response message flag.";
            return false;
        }

        resp.callId_ = std::get<1>(r);

        auto&& error_obj = std::get<2>(r);
        if (!error_obj.is_nil()) {
            resp.error_ = std::make_shared<veigar_msgpack::object_handle>();
            *(resp.error_) = veigar_msgpack::clone(error_obj);
        }

        resp.result_ = std::make_shared<veigar_msgpack::object_handle>(std::get<3>(r), std::move(o.zone()));

        return true;
    } catch (std::exception& e) {
        resp.callId_.clear();
        resp.error_.reset();
        resp.result_.reset();

        exceptionMsg = StringHelper::StringPrintf("An exception occurred during parsing response message: %s.", e.what());
        return false;
    } catch (...) {
        resp.callId_.clear();
        resp.error_.reset();
        resp.result_.reset();

        exceptionMsg = "An exception occurred during parsing response message.";
        return false;
    }
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

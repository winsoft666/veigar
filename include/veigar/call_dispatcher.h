/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DISPATCHER_H_CXIVZD5L
#define DISPATCHER_H_CXIVZD5L
#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include "veigar/config.h"
#include "veigar/msgpack.hpp"
#include "veigar/detail/call.h"
#include "veigar/detail/func_tools.h"
#include "veigar/detail/func_traits.h"
#include "veigar/detail/response.h"
#include "veigar/detail/make_unique.h"

namespace veigar {
class Veigar;
class MessageQueue;

namespace detail {

// This class maintains a registry of functors associated with their names,
// and callable using a msgpack-rpc call pack.
class CallDispatcher {
   public:
    // This functor type unifies the interfaces of functions that are called remotely
    using AdaptorType =
        std::function<std::unique_ptr<veigar_msgpack::object_handle>(veigar_msgpack::object const&)>;

    CallDispatcher(Veigar* veigar) noexcept;
    ~CallDispatcher() noexcept;

    bool init();
    bool isInit() const;
    void uninit();

    std::shared_ptr<MessageQueue> messageQueue();

    // This is the type of messages as per the msgpack-rpc spec.
    // flag(0) - callId - callerChannelName - funcName - args
    using CallMsg = std::tuple<int8_t, std::string, std::string, std::string, veigar_msgpack::object>;

    // Binds a functor to a name so it becomes callable via RPC.
    // name: The name of the functor.
    // func: The functor to bind.
    // F: The type of the functor.
    template <typename F>
    bool bind(std::string const& name, F func);

    // Stores a void, zero-arg functor with a name.
    template <typename F>
    bool bind(std::string const& name, F func, detail::tags::void_result const&, detail::tags::zero_arg const&);

    // Stores a void, non-zero-arg functor with a name.
    template <typename F>
    bool bind(std::string const& name, F func, detail::tags::void_result const&, detail::tags::nonzero_arg const&);

    // Stores a non-void, zero-arg functor with a name.
    template <typename F>
    bool bind(std::string const& name, F func, detail::tags::nonvoid_result const&, detail::tags::zero_arg const&);

    // Stores a non-void, non-zero-arg functor with a name.
    template <typename F>
    bool bind(std::string const& name, F func, detail::tags::nonvoid_result const&, detail::tags::nonzero_arg const&);

    // Unbind a functor with a given name from callable functors.
    void unbind(std::string const& name);

    // returns a list of all names which functors are binded to
    std::vector<std::string> names() const {
        std::vector<std::string> names;
        for (auto it = funcs_.begin(); it != funcs_.end(); ++it)
            names.push_back(it->first);
        return names;
    }

   private:
    // Processes a message that contains a call according to the Msgpack-RPC spec.
    // msg: The messagepack object that contains the call.
    detail::Response dispatch(veigar_msgpack::object const& msg, std::string& callerChannelName);

    bool isFuncNameExist(std::string const& func);

    // Dispatches a call (which will have a response).
    detail::Response dispatchCall(veigar_msgpack::object const& msg, std::string& callerChannelName);

    void dispatchThreadProc();

   private:
    Veigar* veigar_ = nullptr;
    bool init_ = false;

    std::unordered_map<std::string, AdaptorType> funcs_;

    class Impl;
    Impl* impl_ = nullptr;
};
}  // namespace detail
}  // namespace veigar

#include "call_dispatcher.inl"
#endif

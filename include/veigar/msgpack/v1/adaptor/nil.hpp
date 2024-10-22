#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_NIL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_NIL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_NIL_HPP
#define MSGPACK_V1_TYPE_NIL_HPP

#include "veigar/msgpack/v1/adaptor/nil_decl.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace type {

struct nil_t { };

inline bool operator<(nil_t const& lhs, nil_t const& rhs) {
    return &lhs < &rhs;
}

inline bool operator==(nil_t const& lhs, nil_t const& rhs) {
    return &lhs == &rhs;
}

}  // namespace type

namespace adaptor {

template <>
struct convert<type::nil_t> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, type::nil_t&) const {
        if(o.type != veigar_msgpack::type::NIL) { throw veigar_msgpack::type_error(); }
        return o;
    }
};

template <>
struct pack<type::nil_t> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const type::nil_t&) const {
        o.pack_nil();
        return o;
    }
};

template <>
struct object<type::nil_t> {
    void operator()(veigar_msgpack::object& o, type::nil_t) const {
        o.type = veigar_msgpack::type::NIL;
    }
};

template <>
struct object_with_zone<type::nil_t> {
    void operator()(veigar_msgpack::object::with_zone& o, type::nil_t v) const {
        static_cast<veigar_msgpack::object&>(o) << v;
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_NIL_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_NIL_HPP
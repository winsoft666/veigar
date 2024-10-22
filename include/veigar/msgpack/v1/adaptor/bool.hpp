#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_BOOL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_BOOL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2016 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_BOOL_HPP
#define MSGPACK_V1_TYPE_BOOL_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct convert<bool> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, bool& v) const {
        if(o.type != veigar_msgpack::type::BOOLEAN) { throw veigar_msgpack::type_error(); }
        v = o.via.boolean;
        return o;
    }
};

template <>
struct pack<bool> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const bool& v) const {
        if(v) { o.pack_true(); }
        else { o.pack_false(); }
        return o;
    }
};

template <>
struct object<bool> {
    void operator()(veigar_msgpack::object& o, bool v) const {
        o.type = veigar_msgpack::type::BOOLEAN;
        o.via.boolean = v;
    }
};

template <>
struct object_with_zone<bool> {
    void operator()(veigar_msgpack::object::with_zone& o, bool v) const {
        static_cast<veigar_msgpack::object&>(o) << v;
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_BOOL_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_BOOL_HPP
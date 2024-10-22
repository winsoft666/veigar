#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CHAR_PTR_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CHAR_PTR_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_CHAR_PTR_HPP
#define MSGPACK_V1_TYPE_CHAR_PTR_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/object_fwd.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <cstring>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct pack<const char*> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const char* v) const {
        uint32_t size = checked_get_container_size(std::strlen(v));
        o.pack_str(size);
        o.pack_str_body(v, size);
        return o;
    }
};

template <>
struct object_with_zone<const char*> {
    void operator()(veigar_msgpack::object::with_zone& o, const char* v) const {
        uint32_t size = checked_get_container_size(std::strlen(v));
        o.type = veigar_msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.str.ptr = ptr;
        o.via.str.size = size;
        std::memcpy(ptr, v, size);
    }
};

template <>
struct object<const char*> {
    void operator()(veigar_msgpack::object& o, const char* v) const {
        uint32_t size = checked_get_container_size(std::strlen(v));
        o.type = veigar_msgpack::type::STR;
        o.via.str.ptr = v;
        o.via.str.size = size;
    }
};


template <>
struct pack<char*> {
    template <typename Stream>
    packer<Stream>& operator()(packer<Stream>& o, char* v) const {
        return o << static_cast<const char*>(v);
    }
};

template <>
struct object_with_zone<char*> {
    void operator()(veigar_msgpack::object::with_zone& o, char* v) const {
        o << static_cast<const char*>(v);
    }
};

template <>
struct object<char*> {
    void operator()(veigar_msgpack::object& o, char* v) const {
        o << static_cast<const char*>(v);
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_CHAR_PTR_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CHAR_PTR_HPP
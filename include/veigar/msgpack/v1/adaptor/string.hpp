//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_STRING_HPP
#define MSGPACK_V1_TYPE_STRING_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <string>
#include <cstring>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct convert<std::string> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::string& v) const {
        switch (o.type) {
        case veigar_msgpack::type::BIN:
            v.assign(o.via.bin.ptr, o.via.bin.size);
            break;
        case veigar_msgpack::type::STR:
            v.assign(o.via.str.ptr, o.via.str.size);
            break;
        default:
            throw veigar_msgpack::type_error();
            break;
        }
        return o;
    }
};

template <>
struct pack<std::string> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_str(size);
        o.pack_str_body(v.data(), size);
        return o;
    }
};

template <>
struct object<std::string> {
    void operator()(veigar_msgpack::object& o, const std::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = veigar_msgpack::type::STR;
        o.via.str.ptr = v.data();
        o.via.str.size = size;
    }
};

template <>
struct object_with_zone<std::string> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = veigar_msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.str.ptr = ptr;
        o.via.str.size = size;
        std::memcpy(ptr, v.data(), v.size());
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_STRING_HPP

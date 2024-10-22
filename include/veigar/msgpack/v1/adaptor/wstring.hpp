#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_WSTRING_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_WSTRING_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2018 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_WSTRING_HPP
#define MSGPACK_V1_TYPE_WSTRING_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <vector>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined(MSGPACK_USE_CPP03)

template <>
struct as<std::wstring> {
    std::wstring operator()(const veigar_msgpack::object& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        std::wstring v;
        v.reserve(o.via.array.size);
        if (o.via.array.size > 0) {
            veigar_msgpack::object* p = o.via.array.ptr;
            veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
            do {
                v.push_back(p->as<wchar_t>());
                ++p;
            } while (p < pend);
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <>
struct convert<std::wstring> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::wstring& v) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        v.resize(o.via.array.size);
        if (o.via.array.size > 0) {
            veigar_msgpack::object* p = o.via.array.ptr;
            veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
            std::wstring::iterator it = v.begin();
            do {
                p->convert(*it);
                ++p;
                ++it;
            } while(p < pend);
        }
        return o;
    }
};

template <>
struct pack<std::wstring> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::wstring& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for (std::wstring::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <>
struct object_with_zone<std::wstring> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::wstring& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        if (v.empty()) {
            o.via.array.ptr = MSGPACK_NULLPTR;
            o.via.array.size = 0;
        }
        else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
            veigar_msgpack::object* const pend = p + size;
            o.via.array.ptr = p;
            o.via.array.size = size;
            std::wstring::const_iterator it(v.begin());
            do {
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
                *p = veigar_msgpack::object(*it, o.zone);
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_WSTRING_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_WSTRING_HPP
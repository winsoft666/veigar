#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CARRAY_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CARRAY_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_CARRAY_HPP
#define MSGPACK_V1_TYPE_CARRAY_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/object_fwd.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename T, std::size_t N>
struct convert<T[N]> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, T* v) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        if (o.via.array.size > N) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr;
        veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
        do {
            p->convert(*v);
            ++p;
            ++v;
        } while(p < pend);
        return o;
    }
};

template <std::size_t N>
struct convert<char[N]> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, char(&v)[N]) const {
        switch (o.type) {
        case veigar_msgpack::type::BIN:
            if (o.via.bin.size > N) { throw veigar_msgpack::type_error(); }
            std::memcpy(v, o.via.bin.ptr, o.via.bin.size);
            break;
        case veigar_msgpack::type::STR:
            if (o.via.str.size > N) { throw veigar_msgpack::type_error(); }
            std::memcpy(v, o.via.str.ptr, o.via.str.size);
            if (o.via.str.size < N) v[o.via.str.size] = '\0';
            break;
        default:
            throw veigar_msgpack::type_error();
            break;
        }
        return o;
    }
};

template <std::size_t N>
struct convert<unsigned char[N]> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, unsigned char(&v)[N]) const {
        switch (o.type) {
        case veigar_msgpack::type::BIN:
            if (o.via.bin.size > N) { throw veigar_msgpack::type_error(); }
            std::memcpy(v, o.via.bin.ptr, o.via.bin.size);
            break;
        case veigar_msgpack::type::STR:
            if (o.via.str.size > N) { throw veigar_msgpack::type_error(); }
            std::memcpy(v, o.via.str.ptr, o.via.str.size);
            if (o.via.str.size < N) v[o.via.str.size] = '\0';
            break;
        default:
            throw veigar_msgpack::type_error();
            break;
        }
        return o;
    }
};


template <typename T, std::size_t N>
struct pack<T[N]> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const T(&v)[N]) const {
        uint32_t size = checked_get_container_size(N);
        o.pack_array(size);
        const T* ptr = v;
        for (; ptr != &v[N]; ++ptr) o.pack(*ptr);
        return o;
    }
};

template <std::size_t N>
struct pack<char[N]> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const char(&v)[N]) const {
        char const* p = v;
        uint32_t size = checked_get_container_size(N);
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.pack_str(adjusted_size);
        o.pack_str_body(p, adjusted_size);
        return o;
    }
};

template <std::size_t N>
struct pack<const char[N]> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const char(&v)[N]) const {
        uint32_t size = checked_get_container_size(N);
        char const* p = v;
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.pack_str(adjusted_size);
        o.pack_str_body(p, adjusted_size);
        return o;
    }
};

template <std::size_t N>
struct pack<unsigned char[N]> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const unsigned char(&v)[N]) const {
        unsigned char const* p = v;
        uint32_t size = checked_get_container_size(N);
        o.pack_bin(size);
        o.pack_bin_body(reinterpret_cast<char const*>(p), size);
        return o;
    }
};

template <std::size_t N>
struct pack<const unsigned char[N]> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const unsigned char(&v)[N]) const {
        unsigned char const* p = v;
        uint32_t size = checked_get_container_size(N);
        o.pack_bin(size);
        o.pack_bin_body(reinterpret_cast<char const*>(p), size);
        return o;
    }
};

template <typename T, std::size_t N>
struct object_with_zone<T[N]> {
    void operator()(veigar_msgpack::object::with_zone& o, const T(&v)[N]) const {
        uint32_t size = checked_get_container_size(N);
        o.type = veigar_msgpack::type::ARRAY;
        veigar_msgpack::object* ptr = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object) * size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
        o.via.array.ptr = ptr;
        o.via.array.size = size;
        const T* pv = v;
        for (; pv != &v[size]; ++pv) {
            *ptr++ = veigar_msgpack::object(*pv, o.zone);
        }
    }
};

template <std::size_t N>
struct object_with_zone<char[N]> {
    void operator()(veigar_msgpack::object::with_zone& o, const char(&v)[N]) const {
        char const* p = v;
        uint32_t size = checked_get_container_size(N);
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.type = veigar_msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(adjusted_size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.str.ptr = ptr;
        o.via.str.size = adjusted_size;
        std::memcpy(ptr, p, adjusted_size);
    }
};

template <std::size_t N>
struct object_with_zone<const char[N]> {
    void operator()(veigar_msgpack::object::with_zone& o, const char(&v)[N]) const {
        char const* p = v;
        uint32_t size = checked_get_container_size(N);
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.type = veigar_msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(adjusted_size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.str.ptr = ptr;
        o.via.str.size = adjusted_size;
        std::memcpy(ptr, p, adjusted_size);
    }
};

template <std::size_t N>
struct object_with_zone<unsigned char[N]> {
    void operator()(veigar_msgpack::object::with_zone& o, const unsigned char(&v)[N]) const {
        uint32_t size = checked_get_container_size(N);
        o.type = veigar_msgpack::type::BIN;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.bin.ptr = ptr;
        o.via.bin.size = size;
        std::memcpy(ptr, v, size);
    }
};

template <std::size_t N>
struct object_with_zone<const unsigned char[N]> {
    void operator()(veigar_msgpack::object::with_zone& o, const unsigned char(&v)[N]) const {
        uint32_t size = checked_get_container_size(N);
        o.type = veigar_msgpack::type::BIN;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.bin.ptr = ptr;
        o.via.bin.size = size;
        std::memcpy(ptr, v, size);
    }
};

template <std::size_t N>
struct object<char[N]> {
    void operator()(veigar_msgpack::object& o, const char(&v)[N]) const {
        char const* p = v;
        uint32_t size = checked_get_container_size(N);
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.type = veigar_msgpack::type::STR;
        o.via.str.ptr = p;
        o.via.str.size = adjusted_size;
    }
};

template <std::size_t N>
struct object<const char[N]> {
    void operator()(veigar_msgpack::object& o, const char(&v)[N]) const {
        char const* p = v;
        uint32_t size = checked_get_container_size(N);
        char const* p2 = static_cast<char const*>(std::memchr(p, '\0', size));
        uint32_t adjusted_size = p2 ? static_cast<uint32_t>(p2 - p) : size;
        o.type = veigar_msgpack::type::STR;
        o.via.str.ptr = p;
        o.via.str.size = adjusted_size;
    }
};


} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_CARRAY_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CARRAY_HPP
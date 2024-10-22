#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_ARRAY_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_ARRAY_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MSGPACK_V1_TYPE_CPP11_ARRAY_HPP
#define MSGPACK_V1_TYPE_CPP11_ARRAY_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"
#include "veigar/msgpack/meta.hpp"

#include <array>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

namespace detail {

namespace array {

template<typename T, std::size_t N1, std::size_t... I1, std::size_t N2, std::size_t... I2>
inline std::array<T, N1+N2> concat(
    std::array<T, N1>&& a1,
    std::array<T, N2>&& a2,
    veigar_msgpack::seq<I1...>,
    veigar_msgpack::seq<I2...>) {
    return {{ std::move(a1[I1])..., std::move(a2[I2])... }};
}

template<typename T, std::size_t N1, std::size_t N2>
inline std::array<T, N1+N2> concat(std::array<T, N1>&& a1, std::array<T, N2>&& a2) {
    return concat(std::move(a1), std::move(a2), veigar_msgpack::gen_seq<N1>(), veigar_msgpack::gen_seq<N2>());
}

template <typename T, std::size_t N>
struct as_impl {
    static std::array<T, N> as(veigar_msgpack::object const& o) {
        veigar_msgpack::object* p = o.via.array.ptr + N - 1;
        return concat(as_impl<T, N-1>::as(o), std::array<T, 1>{{p->as<T>()}});
    }
};

template <typename T>
struct as_impl<T, 1> {
    static std::array<T, 1> as(veigar_msgpack::object const& o) {
        veigar_msgpack::object* p = o.via.array.ptr;
        return std::array<T, 1>{{p->as<T>()}};
    }
};

template <typename T>
struct as_impl<T, 0> {
    static std::array<T, 0> as(veigar_msgpack::object const&) {
        return std::array<T, 0>();
    }
};

} // namespace array

} // namespace detail

template <typename T, std::size_t N>
struct as<std::array<T, N>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    std::array<T, N> operator()(veigar_msgpack::object const& o) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        if(o.via.array.size > N) { throw veigar_msgpack::type_error(); }
        return detail::array::as_impl<T, N>::as(o);
    }
};

template <typename T, std::size_t N>
struct convert<std::array<T, N>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::array<T, N>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        if(o.via.array.size > N) { throw veigar_msgpack::type_error(); }
        if(o.via.array.size > 0) {
            veigar_msgpack::object* p = o.via.array.ptr;
            veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
            T* it = &v[0];
            do {
                p->convert(*it);
                ++p;
                ++it;
            } while(p < pend);
        }
        return o;
    }
};

template <typename T, std::size_t N>
struct pack<std::array<T, N>> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::array<T, N>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for(auto const& e : v) o.pack(e);
        return o;
    }
};

template <typename T, std::size_t N>
struct object_with_zone<std::array<T, N>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::array<T, N>& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        if(v.empty()) {
            o.via.array.ptr = MSGPACK_NULLPTR;
            o.via.array.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
            o.via.array.size = size;
            o.via.array.ptr = p;
            for (auto const& e : v) *p++ = veigar_msgpack::object(e, o.zone);
        }
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_CPP11_ARRAY_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_ARRAY_HPP
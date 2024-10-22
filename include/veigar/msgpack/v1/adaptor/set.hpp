#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_SET_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_SET_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_SET_HPP
#define MSGPACK_V1_TYPE_SET_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/cpp_version.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <set>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined(MSGPACK_USE_CPP03)

template <typename T, typename Compare, typename Alloc>
struct as<std::set<T, Compare, Alloc>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    std::set<T, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::set<T, Compare, Alloc> v;
        while (p > pbegin) {
            --p;
            v.insert(p->as<T>());
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename T, typename Compare, typename Alloc>
struct convert<std::set<T, Compare, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::set<T, Compare, Alloc>& v) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::set<T, Compare, Alloc> tmp;
        while (p > pbegin) {
            --p;
            tmp.insert(p->as<T>());
        }
#if MSGPACK_CPP_VERSION >= 201103L
        v = std::move(tmp);
#else
        tmp.swap(v);
#endif
        return o;
    }
};

template <typename T, typename Compare, typename Alloc>
struct pack<std::set<T, Compare, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::set<T, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for (typename std::set<T, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename T, typename Compare, typename Alloc>
struct object_with_zone<std::set<T, Compare, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const std::set<T, Compare, Alloc>& v) const {
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
            typename std::set<T, Compare, Alloc>::const_iterator it(v.begin());
            do {
                *p = veigar_msgpack::object(*it, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

#if !defined(MSGPACK_USE_CPP03)

template <typename T, typename Compare, typename Alloc>
struct as<std::multiset<T, Compare, Alloc>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    std::multiset<T, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::multiset<T, Compare, Alloc> v;
        while (p > pbegin) {
            --p;
            v.insert(p->as<T>());
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename T, typename Compare, typename Alloc>
struct convert<std::multiset<T, Compare, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::multiset<T, Compare, Alloc>& v) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::multiset<T, Compare, Alloc> tmp;
        while (p > pbegin) {
            --p;
            tmp.insert(p->as<T>());
        }
#if MSGPACK_CPP_VERSION >= 201103L
        v = std::move(tmp);
#else
        tmp.swap(v);
#endif
        return o;
    }
};

template <typename T, typename Compare, typename Alloc>
struct pack<std::multiset<T, Compare, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::multiset<T, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for (typename std::multiset<T, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename T, typename Compare, typename Alloc>
struct object_with_zone<std::multiset<T, Compare, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const std::multiset<T, Compare, Alloc>& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        if (v.empty()) {
            o.via.array.ptr = MSGPACK_NULLPTR;
            o.via.array.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
            veigar_msgpack::object* const pend = p + size;
            o.via.array.ptr = p;
            o.via.array.size = size;
            typename std::multiset<T, Compare, Alloc>::const_iterator it(v.begin());
            do {
                *p = veigar_msgpack::object(*it, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_SET_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_SET_HPP
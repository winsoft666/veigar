#ifndef __VEIGAR_MSGPACK_ADAPTOR_TR1_UNORDERED_SET_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_ADAPTOR_TR1_UNORDERED_SET_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_TYPE_TR1_UNORDERED_SET_HPP
#define MSGPACK_TYPE_TR1_UNORDERED_SET_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#if defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#define MSGPACK_HAS_STD_UNORDERED_SET
#include <unordered_set>
#define MSGPACK_STD_TR1 std

#else   // defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#if __GNUC__ >= 4

#define MSGPACK_HAS_STD_TR1_UNORDERED_SET

#include <tr1/unordered_set>
#define MSGPACK_STD_TR1 std::tr1

#endif // __GNUC__ >= 4

#endif  // defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#if defined(MSGPACK_STD_TR1)

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename T, typename Hash, typename Compare, typename Alloc>
struct convert<MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc> tmp;
        while(p > pbegin) {
            --p;
            tmp.insert(p->as<T>());
        }
        tmp.swap(v);
        return o;
    }
};

template <typename T, typename Hash, typename Compare, typename Alloc>
struct pack<MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for(typename MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename T, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc>& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        if(v.empty()) {
            o.via.array.ptr = MSGPACK_NULLPTR;
            o.via.array.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
            veigar_msgpack::object* const pend = p + size;
            o.via.array.ptr = p;
            o.via.array.size = size;
            typename MSGPACK_STD_TR1::unordered_set<T, Hash, Compare, Alloc>::const_iterator it(v.begin());
            do {
                *p = veigar_msgpack::object(*it, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};


template <typename T, typename Hash, typename Compare, typename Alloc>
struct convert<MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc> tmp;
        while(p > pbegin) {
            --p;
            tmp.insert(p->as<T>());
        }
        tmp.swap(v);
        return o;
    }
};

template <typename T, typename Hash, typename Compare, typename Alloc>
struct pack<MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for(typename MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename T, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc>& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        if(v.empty()) {
            o.via.array.ptr = MSGPACK_NULLPTR;
            o.via.array.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
            veigar_msgpack::object* const pend = p + size;
            o.via.array.ptr = p;
            o.via.array.size = size;
            typename MSGPACK_STD_TR1::unordered_multiset<T, Hash, Compare, Alloc>::const_iterator it(v.begin());
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
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#undef MSGPACK_STD_TR1

#endif // MSGPACK_STD_TR1

#endif // MSGPACK_TYPE_TR1_UNORDERED_SET_HPP

#endif // !__VEIGAR_MSGPACK_ADAPTOR_TR1_UNORDERED_SET_HPP
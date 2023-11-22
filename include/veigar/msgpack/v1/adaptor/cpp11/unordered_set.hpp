//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_CPP11_UNORDERED_SET_HPP
#define MSGPACK_V1_TYPE_CPP11_UNORDERED_SET_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <unordered_set>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct as<std::unordered_set<Key, Hash, Compare, Alloc>, typename std::enable_if<veigar_msgpack::has_as<Key>::value>::type> {
    std::unordered_set<Key, Hash, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::unordered_set<Key, Hash, Compare, Alloc> v;
        while (p > pbegin) {
            --p;
            v.insert(p->as<Key>());
        }
        return v;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct convert<std::unordered_set<Key, Hash, Compare, Alloc>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::unordered_set<Key, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::unordered_set<Key, Hash, Compare, Alloc> tmp;
        while(p > pbegin) {
            --p;
            tmp.insert(p->as<Key>());
        }
        v = std::move(tmp);
        return o;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct pack<std::unordered_set<Key, Hash, Compare, Alloc>> {
    template <typename Stream>
        veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::unordered_set<Key, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for(typename std::unordered_set<Key, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<std::unordered_set<Key, Hash, Compare, Alloc>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::unordered_set<Key, Hash, Compare, Alloc>& v) const {
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
            typename std::unordered_set<Key, Hash, Compare, Alloc>::const_iterator it(v.begin());
            do {
                *p = veigar_msgpack::object(*it, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};


template <typename Key, typename Hash, typename Compare, typename Alloc>
struct as<std::unordered_multiset<Key, Hash, Compare, Alloc>, typename std::enable_if<veigar_msgpack::has_as<Key>::value>::type> {
    std::unordered_multiset<Key, Hash, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::unordered_multiset<Key, Hash, Compare, Alloc> v;
        while (p > pbegin) {
            --p;
            v.insert(p->as<Key>());
        }
        return v;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct convert<std::unordered_multiset<Key, Hash, Compare, Alloc>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::unordered_multiset<Key, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object* p = o.via.array.ptr + o.via.array.size;
        veigar_msgpack::object* const pbegin = o.via.array.ptr;
        std::unordered_multiset<Key, Hash, Compare, Alloc> tmp;
        while(p > pbegin) {
            --p;
            tmp.insert(p->as<Key>());
        }
        v = std::move(tmp);
        return o;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct pack<std::unordered_multiset<Key, Hash, Compare, Alloc>> {
    template <typename Stream>
        veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::unordered_multiset<Key, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for(typename std::unordered_multiset<Key, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename Key, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<std::unordered_multiset<Key, Hash, Compare, Alloc>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::unordered_multiset<Key, Hash, Compare, Alloc>& v) const {
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
            typename std::unordered_multiset<Key, Hash, Compare, Alloc>::const_iterator it(v.begin());
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

#endif // MSGPACK_V1_TYPE_CPP11_UNORDERED_SET_HPP

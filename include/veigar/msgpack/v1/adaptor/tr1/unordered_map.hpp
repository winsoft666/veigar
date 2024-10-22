#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_TR1_UNORDERED_MAP_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_TR1_UNORDERED_MAP_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_TYPE_TR1_UNORDERED_MAP_HPP
#define MSGPACK_TYPE_TR1_UNORDERED_MAP_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#if defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#define MSGPACK_HAS_STD_UNORDERED_MAP
#include <unordered_map>
#define MSGPACK_STD_TR1 std

#else   // defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#if __GNUC__ >= 4

#define MSGPACK_HAS_STD_TR1_UNORDERED_MAP

#include <tr1/unordered_map>
#define MSGPACK_STD_TR1 std::tr1

#endif // __GNUC__ >= 4

#endif  // defined(_LIBCPP_VERSION) || (_MSC_VER >= 1700)

#if defined(MSGPACK_STD_TR1)

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct convert<MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc> tmp;
        for(; p != pend; ++p) {
            K key;
            p->key.convert(key);
            p->val.convert(tmp[key]);
        }
        tmp.swap(v);
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct pack<MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for(typename MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct object_with_zone<MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc>& v) const {
        o.type = veigar_msgpack::type::MAP;
        if(v.empty()) {
            o.via.map.ptr  = MSGPACK_NULLPTR;
            o.via.map.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object_kv* p = static_cast<veigar_msgpack::object_kv*>(o.zone.allocate_align(sizeof(veigar_msgpack::object_kv)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object_kv)));
            veigar_msgpack::object_kv* const pend = p + size;
            o.via.map.ptr  = p;
            o.via.map.size = size;
            typename MSGPACK_STD_TR1::unordered_map<K, V, Hash, Pred, Alloc>::const_iterator it(v.begin());
            do {
                p->key = veigar_msgpack::object(it->first, o.zone);
                p->val = veigar_msgpack::object(it->second, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct convert<MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc> tmp;
        for(; p != pend; ++p) {
            std::pair<K, V> value;
            p->key.convert(value.first);
            p->val.convert(value.second);
            tmp.insert(value);
        }
        tmp.swap(v);
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct pack<MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for(typename MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Pred, typename Alloc>
struct object_with_zone<MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc>& v) const {
        o.type = veigar_msgpack::type::MAP;
        if(v.empty()) {
            o.via.map.ptr  = MSGPACK_NULLPTR;
            o.via.map.size = 0;
        } else {
            uint32_t size = checked_get_container_size(v.size());
            veigar_msgpack::object_kv* p = static_cast<veigar_msgpack::object_kv*>(o.zone.allocate_align(sizeof(veigar_msgpack::object_kv)*size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object_kv)));
            veigar_msgpack::object_kv* const pend = p + size;
            o.via.map.ptr  = p;
            o.via.map.size = size;
            typename MSGPACK_STD_TR1::unordered_multimap<K, V, Hash, Pred, Alloc>::const_iterator it(v.begin());
            do {
                p->key = veigar_msgpack::object(it->first, o.zone);
                p->val = veigar_msgpack::object(it->second, o.zone);
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

#endif // MSGPACK_TYPE_TR1_UNORDERED_MAP_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_TR1_UNORDERED_MAP_HPP
#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_UNORDERED_MAP_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_UNORDERED_MAP_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_CPP11_UNORDERED_MAP_HPP
#define MSGPACK_V1_TYPE_CPP11_UNORDERED_MAP_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <unordered_map>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct as<
    std::unordered_map<K, V, Hash, Compare, Alloc>,
    typename std::enable_if<veigar_msgpack::has_as<K>::value || veigar_msgpack::has_as<V>::value>::type> {
    std::unordered_map<K, V, Hash, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        std::unordered_map<K, V, Hash, Compare, Alloc> v;
        for (; p != pend; ++p) {
            v.emplace(p->key.as<K>(), p->val.as<V>());
        }
        return v;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct convert<std::unordered_map<K, V, Hash, Compare, Alloc>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::unordered_map<K, V, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        std::unordered_map<K, V, Hash, Compare, Alloc> tmp;
        for(; p != pend; ++p) {
            K key;
            p->key.convert(key);
            p->val.convert(tmp[std::move(key)]);
        }
        v = std::move(tmp);
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct pack<std::unordered_map<K, V, Hash, Compare, Alloc>> {
    template <typename Stream>
        veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::unordered_map<K, V, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for(typename std::unordered_map<K, V, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<std::unordered_map<K, V, Hash, Compare, Alloc>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::unordered_map<K, V, Hash, Compare, Alloc>& v) const {
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
            typename std::unordered_map<K, V, Hash, Compare, Alloc>::const_iterator it(v.begin());
            do {
                p->key = veigar_msgpack::object(it->first, o.zone);
                p->val = veigar_msgpack::object(it->second, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};


template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct as<
    std::unordered_multimap<K, V, Hash, Compare, Alloc>,
    typename std::enable_if<veigar_msgpack::has_as<K>::value || veigar_msgpack::has_as<V>::value>::type> {
    std::unordered_multimap<K, V, Hash, Compare, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        std::unordered_multimap<K, V, Hash, Compare, Alloc> v;
        for (; p != pend; ++p) {
            v.emplace(p->key.as<K>(), p->val.as<V>());
        }
        return v;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct convert<std::unordered_multimap<K, V, Hash, Compare, Alloc>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::unordered_multimap<K, V, Hash, Compare, Alloc>& v) const {
        if(o.type != veigar_msgpack::type::MAP) { throw veigar_msgpack::type_error(); }
        veigar_msgpack::object_kv* p(o.via.map.ptr);
        veigar_msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        std::unordered_multimap<K, V, Hash, Compare, Alloc> tmp;
        for(; p != pend; ++p) {
            std::pair<K, V> value;
            p->key.convert(value.first);
            p->val.convert(value.second);
            tmp.insert(std::move(value));
        }
        v = std::move(tmp);
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct pack<std::unordered_multimap<K, V, Hash, Compare, Alloc>> {
    template <typename Stream>
        veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::unordered_multimap<K, V, Hash, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for(typename std::unordered_multimap<K, V, Hash, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Hash, typename Compare, typename Alloc>
struct object_with_zone<std::unordered_multimap<K, V, Hash, Compare, Alloc>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::unordered_multimap<K, V, Hash, Compare, Alloc>& v) const {
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
            typename std::unordered_multimap<K, V, Hash, Compare, Alloc>::const_iterator it(v.begin());
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


#endif // MSGPACK_V1_TYPE_CPP11_UNORDERED_MAP_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_UNORDERED_MAP_HPP
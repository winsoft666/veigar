#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_LIST_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_LIST_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_LIST_HPP
#define MSGPACK_V1_TYPE_LIST_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <list>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined(MSGPACK_USE_CPP03)

template <typename T, typename Alloc>
struct as<std::list<T, Alloc>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    std::list<T, Alloc> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        std::list<T, Alloc> v;
        veigar_msgpack::object* p = o.via.array.ptr;
        veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
        for (; p < pend; ++p) {
            v.push_back(p->as<T>());
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename T, typename Alloc>
struct convert<std::list<T, Alloc> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::list<T, Alloc>& v) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        v.resize(o.via.array.size);
        veigar_msgpack::object* p = o.via.array.ptr;
        veigar_msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
        typename std::list<T, Alloc>::iterator it = v.begin();
        for (; p < pend; ++p, ++it) {
            p->convert(*it);
        }
        return o;
    }
};

template <typename T, typename Alloc>
struct pack<std::list<T, Alloc> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::list<T, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_array(size);
        for (typename std::list<T, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(*it);
        }
        return o;
    }
};

template <typename T, typename Alloc>
struct object_with_zone<std::list<T, Alloc> > {
    void operator()(veigar_msgpack::object::with_zone& o, const std::list<T, Alloc>& v) const {
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
            typename std::list<T, Alloc>::const_iterator it(v.begin());
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

#endif // MSGPACK_V1_TYPE_LIST_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_LIST_HPP
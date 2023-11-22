//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2009 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_PAIR_HPP
#define MSGPACK_V1_TYPE_PAIR_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/meta.hpp"

#include <utility>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined(MSGPACK_USE_CPP03)

template <typename T1, typename T2>
struct as<std::pair<T1, T2>,
          typename std::enable_if<veigar_msgpack::any_of<veigar_msgpack::has_as, T1, T2>::value>::type> {
    std::pair<T1, T2> operator()(veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        if (o.via.array.size != 2) { throw veigar_msgpack::type_error(); }
        return std::make_pair(o.via.array.ptr[0].as<T1>(), o.via.array.ptr[1].as<T2>());
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename T1, typename T2>
struct convert<std::pair<T1, T2> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::pair<T1, T2>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        if(o.via.array.size != 2) { throw veigar_msgpack::type_error(); }
        o.via.array.ptr[0].convert(v.first);
        o.via.array.ptr[1].convert(v.second);
        return o;
    }
};

template <typename T1, typename T2>
struct pack<std::pair<T1, T2> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::pair<T1, T2>& v) const {
        o.pack_array(2);
        o.pack(v.first);
        o.pack(v.second);
        return o;
    }
};

template <typename T1, typename T2>
struct object_with_zone<std::pair<T1, T2> > {
    void operator()(veigar_msgpack::object::with_zone& o, const std::pair<T1, T2>& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        veigar_msgpack::object* p = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*2, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
        o.via.array.ptr = p;
        o.via.array.size = 2;
        p[0] = veigar_msgpack::object(v.first, o.zone);
        p[1] = veigar_msgpack::object(v.second, o.zone);
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_PAIR_HPP

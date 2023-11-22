//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2017 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_OPTIONAL_HPP
#define MSGPACK_V1_TYPE_OPTIONAL_HPP

#include "veigar/msgpack/cpp_version.hpp"

#if MSGPACK_CPP_VERSION >= 201703

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <optional>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined (MSGPACK_USE_CPP03)

template <typename T>
struct as<std::optional<T>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    std::optional<T> operator()(veigar_msgpack::object const& o) const {
        if(o.is_nil()) return std::nullopt;
        return o.as<T>();
    }
};

#endif // !defined (MSGPACK_USE_CPP03)

template <typename T>
struct convert<std::optional<T> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::optional<T>& v) const {
        if(o.is_nil()) v = std::nullopt;
        else {
            T t;
            veigar_msgpack::adaptor::convert<T>()(o, t);
            v = t;
        }
        return o;
    }
};

template <typename T>
struct pack<std::optional<T> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::optional<T>& v) const {
        if (v) o.pack(*v);
        else o.pack_nil();
        return o;
    }
};

template <typename T>
struct object<std::optional<T> > {
    void operator()(veigar_msgpack::object& o, const std::optional<T>& v) const {
        if (v) veigar_msgpack::adaptor::object<T>()(o, *v);
        else o.type = veigar_msgpack::type::NIL;
    }
};

template <typename T>
struct object_with_zone<std::optional<T> > {
    void operator()(veigar_msgpack::object::with_zone& o, const std::optional<T>& v) const {
        if (v) veigar_msgpack::adaptor::object_with_zone<T>()(o, *v);
        else o.type = veigar_msgpack::type::NIL;
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_CPP_VERSION >= 201703

#endif // MSGPACK_V1_TYPE_OPTIONAL_HPP

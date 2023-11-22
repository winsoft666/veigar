//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_BOOST_OPTIONAL_HPP
#define MSGPACK_V1_TYPE_BOOST_OPTIONAL_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif // defined(__GNUC__)

// To suppress warning on Boost.1.58.0
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)

#include <boost/optional.hpp>

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__)

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

#if !defined (MSGPACK_USE_CPP03)

template <typename T>
struct as<boost::optional<T>, typename std::enable_if<veigar_msgpack::has_as<T>::value>::type> {
    boost::optional<T> operator()(veigar_msgpack::object const& o) const {
        if(o.is_nil()) return boost::none;
        return o.as<T>();
    }
};

#endif // !defined (MSGPACK_USE_CPP03)

template <typename T>
struct convert<boost::optional<T> > {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, boost::optional<T>& v) const {
        if(o.is_nil()) v = boost::none;
        else {
            T t;
            veigar_msgpack::adaptor::convert<T>()(o, t);
            v = t;
        }
        return o;
    }
};

template <typename T>
struct pack<boost::optional<T> > {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const boost::optional<T>& v) const {
        if (v) o.pack(*v);
        else o.pack_nil();
        return o;
    }
};

template <typename T>
struct object<boost::optional<T> > {
    void operator()(veigar_msgpack::object& o, const boost::optional<T>& v) const {
        if (v) veigar_msgpack::adaptor::object<T>()(o, *v);
        else o.type = veigar_msgpack::type::NIL;
    }
};

template <typename T>
struct object_with_zone<boost::optional<T> > {
    void operator()(veigar_msgpack::object::with_zone& o, const boost::optional<T>& v) const {
        if (v) veigar_msgpack::adaptor::object_with_zone<T>()(o, *v);
        else o.type = veigar_msgpack::type::NIL;
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_BOOST_OPTIONAL_HPP

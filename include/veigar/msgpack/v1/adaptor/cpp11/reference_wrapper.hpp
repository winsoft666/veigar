#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_REFERENCE_WRAPPER_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_REFERENCE_WRAPPER_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MSGPACK_V1_TYPE_CPP11_REFERENCE_WRAPPER_HPP
#define MSGPACK_V1_TYPE_CPP11_REFERENCE_WRAPPER_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"

#include <memory>
#include <type_traits>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <typename T>
struct convert<std::reference_wrapper<T>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::reference_wrapper<T>& v) const {
        veigar_msgpack::adaptor::convert<T>()(o, v.get());
        return o;
    }
};

template <typename T>
struct pack<std::reference_wrapper<T>> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& o, const std::reference_wrapper<T>& v) const {
        o.pack(v.get());
        return o;
    }
};

template <typename T>
struct object<std::reference_wrapper<T> > {
    void operator()(veigar_msgpack::object& o, const std::reference_wrapper<T>& v) const {
        veigar_msgpack::adaptor::object<typename std::remove_const<T>::type>()(o, v.get());
    }
};

template <typename T>
struct object_with_zone<std::reference_wrapper<T>> {
    void operator()(veigar_msgpack::object::with_zone& o, const std::reference_wrapper<T>& v) const {
        veigar_msgpack::adaptor::object_with_zone<typename std::remove_const<T>::type>()(o, v.get());
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_CPP11_REFERENCE_WRAPPER_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CPP11_REFERENCE_WRAPPER_HPP
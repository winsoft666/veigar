#ifndef __VEIGAR_MSGPACK_V2_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V2_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V2_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP
#define MSGPACK_V2_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP

#include "veigar/msgpack/v1/adaptor/boost/msgpack_variant_decl.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v2) {
/// @endcond

namespace type {

using v1::type::basic_variant;
using v1::type::variant;
using v1::type::variant_ref;

using v1::type::operator<;

using v1::type::operator==;

} // namespace type

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v2)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V2_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP

#endif // !__VEIGAR_MSGPACK_V2_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP
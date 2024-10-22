#ifndef __VEIGAR_MSGPACK_V2_ADAPTOR_INT_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V2_ADAPTOR_INT_DECL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V2_TYPE_INT_DECL_HPP
#define MSGPACK_V2_TYPE_INT_DECL_HPP

#include "veigar/msgpack/v1/adaptor/int_decl.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v2){
/// @endcond

namespace type {
namespace detail {


template <typename T, bool Signed>
struct convert_integer_sign;

template <typename T>
struct is_signed;


template <bool Signed>
struct object_sign;

//using v1::type::detail::convert_integer_sign;

//using v1::type::detail::is_signed;

using v1::type::detail::convert_integer;

//using v1::type::detail::object_char_sign;

using v1::type::detail::object_char;

}  // namespace detail
}  // namespace type

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v2)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V2_TYPE_INT_DECL_HPP

#endif // !__VEIGAR_MSGPACK_V2_ADAPTOR_INT_DECL_HPP
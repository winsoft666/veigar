#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015-2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP
#define MSGPACK_V1_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/adaptor/check_container_size.hpp"
#include "veigar/msgpack/adaptor/boost/string_ref.hpp"
#include "veigar/msgpack/adaptor/ext.hpp"
#include "veigar/msgpack/adaptor/raw.hpp"

#include <string>
#include <vector>
namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace type {

template <typename STR, typename BIN, typename EXT>
struct basic_variant;

template <typename STR, typename BIN, typename EXT>
bool operator<(basic_variant<STR, BIN, EXT> const& lhs, basic_variant<STR, BIN, EXT> const& rhs);

template <typename STR, typename BIN, typename EXT>
bool operator==(basic_variant<STR, BIN, EXT> const& lhs, basic_variant<STR, BIN, EXT> const& rhs);

template <typename STR, typename BIN, typename EXT>
bool operator!=(basic_variant<STR, BIN, EXT> const& lhs, basic_variant<STR, BIN, EXT> const& rhs);

typedef basic_variant<std::string, std::vector<char>, veigar_msgpack::type::ext> variant;
typedef basic_variant<
#if (BOOST_VERSION / 100000) >= 1 && ((BOOST_VERSION / 100) % 1000) >= 53
    boost::string_ref,
#else  // (BOOST_VERSION / 100000) >= 1 && ((BOOST_VERSION / 100) % 1000) >= 53
    std::string,
#endif // (BOOST_VERSION / 100000) >= 1 && ((BOOST_VERSION / 100) % 1000) >= 53
    veigar_msgpack::type::raw_ref, veigar_msgpack::type::ext_ref> variant_ref;

} // namespace type

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace veigar_msgpack

#endif // MSGPACK_V1_TYPE_BOOST_MSGPACK_VARIANT_DECL_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_BOOST_MSGPACK_VARIANT_DECL_HPP
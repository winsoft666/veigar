//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2014 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_OBJECT_DECL_HPP
#define MSGPACK_V1_OBJECT_DECL_HPP

#include "veigar/msgpack/versioning.hpp"
#include "veigar/msgpack/pack.hpp"
#include "veigar/msgpack/zone.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"

#include <cstring>
#include <stdexcept>
#include <typeinfo>
#include <limits>
#include <ostream>
#include <typeinfo>
#include <iomanip>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

/// The class holds object and zone
class object_handle;

namespace detail {

template <std::size_t N>
std::size_t add_ext_type_size(std::size_t size);

template <>
std::size_t add_ext_type_size<4>(std::size_t size);

} // namespace detail

std::size_t aligned_zone_size(veigar_msgpack::object const& obj);

/// clone object
/**
 * Clone (deep copy) object.
 * The copied object is located on newly allocated zone.
 * @param obj copy source object
 *
 * @return object_handle that holds deep copied object and zone.
 */
object_handle clone(veigar_msgpack::object const& obj);

namespace detail {

template <typename Stream, typename T>
struct packer_serializer;

} // namespace detail

// obsolete
template <typename Type>
class define;

bool operator==(const veigar_msgpack::object& x, const veigar_msgpack::object& y);

template <typename T>
bool operator==(const veigar_msgpack::object& x, const T& y);

bool operator!=(const veigar_msgpack::object& x, const veigar_msgpack::object& y);

template <typename T>
bool operator==(const T& y, const veigar_msgpack::object& x);

template <typename T>
bool operator!=(const veigar_msgpack::object& x, const T& y);

template <typename T>
bool operator!=(const T& y, const veigar_msgpack::object& x);

class object_parser;

template <typename Stream>
struct object_pack_visitor;

struct object_stringize_visitor;

// obsolete
template <typename T>
MSGPACK_DEPRECATED("please use member function version of object::convert(T&)")
void convert(T& v, veigar_msgpack::object const& o);

// obsolete
template <typename Stream, typename T>
MSGPACK_DEPRECATED("please use member function version of packer::pack(const T&)")
void pack(veigar_msgpack::packer<Stream>& o, const T& v);

// obsolete
template <typename Stream, typename T>
MSGPACK_DEPRECATED("please use member function version of packer::pack(const T&)")
void pack_copy(veigar_msgpack::packer<Stream>& o, T v);

template <typename Stream>
veigar_msgpack::packer<Stream>& operator<< (veigar_msgpack::packer<Stream>& o, const veigar_msgpack::object& v);

template <typename Stream>
veigar_msgpack::packer<Stream>& operator<< (veigar_msgpack::packer<Stream>& o, const veigar_msgpack::object::with_zone& v);

std::ostream& operator<< (std::ostream& s, const veigar_msgpack::object& v);

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_OBJECT_DECL_HPP

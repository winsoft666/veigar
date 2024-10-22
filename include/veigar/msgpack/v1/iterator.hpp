#ifndef __VEIGAR_MSGPACK_V1_ITERATOR_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ITERATOR_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015-2016 MIZUKI Hirata
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MSGPACK_V1_ITERATOR_HPP
#define MSGPACK_V1_ITERATOR_HPP
#if !defined(MSGPACK_USE_CPP03)

#include "veigar/msgpack/v1/fbuffer_decl.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

inline veigar_msgpack::object_kv* begin(veigar_msgpack::object_map &map) { return map.ptr; }
inline const veigar_msgpack::object_kv* begin(const veigar_msgpack::object_map &map) { return map.ptr; }
inline veigar_msgpack::object_kv* end(veigar_msgpack::object_map &map) { return map.ptr + map.size; }
inline const veigar_msgpack::object_kv* end(const veigar_msgpack::object_map &map) { return map.ptr + map.size; }

inline veigar_msgpack::object* begin(veigar_msgpack::object_array &array) { return array.ptr; }
inline const veigar_msgpack::object* begin(const veigar_msgpack::object_array &array) { return array.ptr; }
inline veigar_msgpack::object* end(veigar_msgpack::object_array &array) { return array.ptr + array.size; }
inline const veigar_msgpack::object* end(const veigar_msgpack::object_array &array) { return array.ptr + array.size; }

/// @cond
}
/// @endcond

}

#endif // !defined(MSGPACK_USE_CPP03)
#endif // MSGPACK_V1_ITERATOR_HPP

#endif // !__VEIGAR_MSGPACK_V1_ITERATOR_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015-2016 MIZUKI Hirata
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MSGPACK_V1_ITERATOR_DECL_HPP
#define MSGPACK_V1_ITERATOR_DECL_HPP
#if !defined(MSGPACK_USE_CPP03)

#include "veigar/msgpack/object_fwd.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

veigar_msgpack::object_kv* begin(veigar_msgpack::object_map &map);
const veigar_msgpack::object_kv* begin(const veigar_msgpack::object_map &map);
veigar_msgpack::object_kv* end(veigar_msgpack::object_map &map);
const veigar_msgpack::object_kv* end(const veigar_msgpack::object_map &map);

veigar_msgpack::object* begin(veigar_msgpack::object_array &array);
const veigar_msgpack::object* begin(const veigar_msgpack::object_array &array);
veigar_msgpack::object* end(veigar_msgpack::object_array &array);
const veigar_msgpack::object* end(const veigar_msgpack::object_array &array);

/// @cond
}
/// @endcond

}

#endif // !defined(MSGPACK_USE_CPP03)
#endif // MSGPACK_V1_ITERATOR_DECL_HPP

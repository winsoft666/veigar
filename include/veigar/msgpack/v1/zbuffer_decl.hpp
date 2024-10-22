#ifndef __VEIGAR_MSGPACK_V1_ZBUFFER_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ZBUFFER_DECL_HPP
//
// MessagePack for C++ deflate buffer implementation
//
// Copyright (C) 2010-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_ZBUFFER_DECL_HPP
#define MSGPACK_V1_ZBUFFER_DECL_HPP

#include "veigar/msgpack/versioning.hpp"

#ifndef MSGPACK_ZBUFFER_RESERVE_SIZE
#define MSGPACK_ZBUFFER_RESERVE_SIZE 512
#endif

#ifndef MSGPACK_ZBUFFER_INIT_SIZE
#define MSGPACK_ZBUFFER_INIT_SIZE 8192
#endif

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

class zbuffer;

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_ZBUFFER_DECL_HPP

#endif // !__VEIGAR_MSGPACK_V1_ZBUFFER_DECL_HPP
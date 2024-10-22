#ifndef __VEIGAR_MSGPACK_V1_VREFBUFFER_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_VREFBUFFER_DECL_HPP
//
// MessagePack for C++ zero-copy buffer implementation
//
// Copyright (C) 2008-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_VREFBUFFER_DECL_HPP
#define MSGPACK_V1_VREFBUFFER_DECL_HPP

#include "veigar/msgpack/versioning.hpp"

#include <cstdlib>

#ifndef MSGPACK_VREFBUFFER_REF_SIZE
#define MSGPACK_VREFBUFFER_REF_SIZE 32
#endif

#ifndef MSGPACK_VREFBUFFER_CHUNK_SIZE
#define MSGPACK_VREFBUFFER_CHUNK_SIZE 8192
#endif

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

class vrefbuffer;

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_VREFBUFFER_DECL_HPP

#endif // !__VEIGAR_MSGPACK_V1_VREFBUFFER_DECL_HPP
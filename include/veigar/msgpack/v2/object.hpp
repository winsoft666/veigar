//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V2_OBJECT_HPP
#define MSGPACK_V2_OBJECT_HPP

#include "veigar/msgpack/object_fwd.hpp"


namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v2) {
/// @endcond

inline object::implicit_type object::convert() const
{
    return v1::object::convert();
}

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v2)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V2_OBJECT_HPP

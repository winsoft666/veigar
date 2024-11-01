#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_DEFINE_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_DEFINE_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2014 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_DEFINE_HPP
#define MSGPACK_V1_DEFINE_HPP

#if defined(MSGPACK_USE_CPP03)
#include "veigar/msgpack/v1/adaptor/detail/cpp03_define_array.hpp"
#include "veigar/msgpack/v1/adaptor/detail/cpp03_define_map.hpp"
#else  // MSGPACK_USE_CPP03
#include "veigar/msgpack/v1/adaptor/detail/cpp11_define_array.hpp"
#include "veigar/msgpack/v1/adaptor/detail/cpp11_define_map.hpp"
#endif // MSGPACK_USE_CPP03

#endif // MSGPACK_V1_DEFINE_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_DEFINE_HPP
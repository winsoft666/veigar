//
// MessagePack for C++
//
// Copyright (C) 2008-2009 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/iterator.hpp"
#include "veigar/msgpack/zone.hpp"
#include "veigar/msgpack/pack.hpp"
#include "veigar/msgpack/null_visitor.hpp"
#include "veigar/msgpack/parse.hpp"
#include "veigar/msgpack/unpack.hpp"
#include "veigar/msgpack/x3_parse.hpp"
#include "veigar/msgpack/x3_unpack.hpp"
#include "veigar/msgpack/sbuffer.hpp"
#include "veigar/msgpack/vrefbuffer.hpp"
#include "veigar/msgpack/version.hpp"
#include "veigar/msgpack/type.hpp"

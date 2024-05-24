/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
#ifndef VEIGAR_CONFIG_H_
#define VEIGAR_CONFIG_H_
#pragma once

namespace veigar {

static constexpr unsigned VERSION_MAJOR = 1;
static constexpr unsigned VERSION_MINOR = 3;

}  // namespace veigar

#ifndef MSGPACK_NO_BOOST
#define MSGPACK_NO_BOOST
#endif

#ifndef VEIGAR_DISPATCHER_THREAD_NUMBER
#define VEIGAR_DISPATCHER_THREAD_NUMBER 6
#endif

#endif  // !VEIGAR_CONFIG_H_
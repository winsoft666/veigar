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
static constexpr unsigned VERSION_MINOR = 2;

}  // namespace veigar

#ifndef MSGPACK_NO_BOOST
#define MSGPACK_NO_BOOST
#endif

// The maximum number of messages in the message queue.
// When push message to the queue, if the total number of messages is greater than this value, the first message will be discarded.
//
#ifndef VEIGAR_MAX_MESSAGE_NUMBER
#define VEIGAR_MAX_MESSAGE_NUMBER 10
#endif

// The maximum number of bytes expected for a single message.
// The total shared memory size is VEIGAR_MAX_MESSAGE_NUMBER * VEIGAR_MAX_MESSAGE_EXPECTED_SIZE. 
// If the size of a single message is greater than VEIGAR_MAX_MESSAGE_EXPECTED_SIZE, the message can still be sent,
// but if the size of a single message is greater than VEIGAR_MAX_MESSAGE_NUMBER * VEIGAR_MAX_MESSAGE_EXPECTED_SIZE, it will fail.
//
#ifndef VEIGAR_MAX_MESSAGE_EXPECTED_SIZE
#define VEIGAR_MAX_MESSAGE_EXPECTED_SIZE 1048576  // 1MB
#endif

#ifndef VEIGAR_DISPATCHER_THREAD_NUMBER
#define VEIGAR_DISPATCHER_THREAD_NUMBER 6
#endif

#endif  // !VEIGAR_CONFIG_H_
/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023 winsoft666 <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
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
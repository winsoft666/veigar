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

#ifdef VEIGAR_STATIC
#define VEIGAR_API
#else
#if defined(VEIGAR_EXPORTS)
#if defined(_MSC_VER)
#define VEIGAR_API __declspec(dllexport)
#else
#define VEIGAR_API
#endif
#else
#if defined(_MSC_VER)
#define VEIGAR_API __declspec(dllimport)
#else
#define VEIGAR_API
#endif
#endif
#endif

namespace veigar {

static constexpr unsigned VERSION_MAJOR = 1;
static constexpr unsigned VERSION_MINOR = 5;

}  // namespace veigar

#ifndef MSGPACK_NO_BOOST
#define MSGPACK_NO_BOOST
#endif

#ifndef VEIGAR_CALL_QUEUE_NAME_SUFFIX
#define VEIGAR_CALL_QUEUE_NAME_SUFFIX "_call"
#endif

#ifndef VEIGAR_RESPONSE_QUEUE_NAME_SUFFIX
#define VEIGAR_RESPONSE_QUEUE_NAME_SUFFIX "_resp"
#endif

#ifndef VEIGAR_DISPATCHER_THREAD_NUMBER
#define VEIGAR_DISPATCHER_THREAD_NUMBER 3
#endif

#ifndef VEIGAR_SEND_CALL_THREAD_NUMBER
#define VEIGAR_SEND_CALL_THREAD_NUMBER 3
#endif

#ifndef VEIGAR_SEND_RESPONSE_THREAD_NUMBER
#define VEIGAR_SEND_RESPONSE_THREAD_NUMBER 3
#endif

#ifndef VEIGAR_WRITE_RESPONSE_QUEUE_TIMEOUT
#define VEIGAR_WRITE_RESPONSE_QUEUE_TIMEOUT 1500 // ms
#endif

#endif  // !VEIGAR_CONFIG_H_
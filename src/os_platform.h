/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_PLATFORM_H_
#define VEIGAR_PLATFORM_H_
#pragma once

// detect platform

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) ||                    \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WINCE) || defined(_WIN32_WCE)
#define VEIGAR_OS_WINDOWS 1
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define VEIGAR_OS_LINUX 1
#elif defined(__QNX__)
#define VEIGAR_OS_QNX 1
#elif defined(__APPLE__)
#define VEIGAR_OS_MACOS 1
#elif defined(__unix__) || defined(unix) || defined(__unix)
#define VEIGAR_OS_UNIX 1
#endif
#endif  // !VEIGAR_PLATFORM_H_

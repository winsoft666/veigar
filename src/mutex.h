/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_MUTEX_H_
#define VEIGAR_MUTEX_H_
#pragma once

#include "os_platform.h"
#include <inttypes.h>
#ifdef VEIGAR_OS_WINDOWS
#include <cstdint>
#include <system_error>
#include <Windows.h>
#else
#include <cstring>
#include <cassert>
#include <cstdint>
#include <system_error>
#include <time.h>
#include <pthread.h>
#endif

namespace veigar {
struct MutexHandle {
#ifdef VEIGAR_OS_WINDOWS
    HANDLE pMutex;
#else
    pthread_mutex_t pMutex;
#endif
};

class Mutex {
   public:
    Mutex() noexcept;
    ~Mutex() noexcept;

    bool valid() const noexcept;

    bool open(const char* name) noexcept;
    void close() noexcept;

    bool lock(int64_t ms) noexcept;
    bool tryLock() noexcept;
    bool unlock() noexcept;

   private:
    MutexHandle* h_ = nullptr;
};
}  // namespace veigar
#endif
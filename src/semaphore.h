/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_SEMAPHORE_H_
#define VEIGAR_SEMAPHORE_H_
#pragma once

#include "os_platform.h"
#include <inttypes.h>
#include <string>
#ifdef VEIGAR_OS_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#endif

namespace veigar {
struct SemaphoreHandle {
#ifdef VEIGAR_OS_WINDOWS
    HANDLE h_ = NULL;
#else
    sem_t* named_ = SEM_FAILED;  // Named
    sem_t unnamed_;              // Unnamed
#endif
};
class Semaphore {
   public:
    Semaphore() noexcept;
    ~Semaphore() noexcept;

    bool open(const std::string& name, int value = 0) noexcept;
    void close() noexcept;

    bool valid() const noexcept;

    void wait() noexcept;                   // semaphore - 1
    bool wait(const int64_t& ms) noexcept;  // semaphore - 1 , timeout ms
    void release() noexcept;                // semaphore + 1

   private:
    bool named_ = false;
    SemaphoreHandle* sh_ = nullptr;
};
}  // namespace veigar
#endif
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
#include <time.h>
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
    Semaphore() noexcept = default;
    ~Semaphore() noexcept = default;

    static bool IsExist(const std::string& name);

    // The semaphore will be created if it does not already exist.
    // 
    // unnamed semaphore used in synchronization among threads
    // named semaphore used in synchronization among processes.
    bool open(const std::string& name, int value = 0, int maxValue = 2147483647);
    void close();

    bool valid() const;

    void wait();                   // semaphore - 1
    bool wait(const int64_t& ms);  // semaphore - 1 , timeout ms
    void release();                // semaphore + 1

   private:
    bool named_ = false;
    SemaphoreHandle* sh_ = nullptr;
};
}  // namespace veigar
#endif
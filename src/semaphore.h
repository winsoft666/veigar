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
class Semaphore {
   public:
    Semaphore();
    ~Semaphore();

    bool open(const std::string& name, int value = 0) noexcept;
    void close() noexcept;

    bool valid() const noexcept;

    void wait();                   // semaphore - 1
    bool wait(const int64_t& ms);  // semaphore - 1 , timeout ms
    void release();                // semaphore + 1

   private:
#ifdef VEIGAR_OS_WINDOWS
    HANDLE h_ = NULL;
#else
    sem_t* h_ = SEM_FAILED;
#endif
};
}  // namespace veigar
#endif
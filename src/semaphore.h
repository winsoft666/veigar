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
    Semaphore();
    ~Semaphore();

    bool open(const std::string& name, int value = 0) noexcept;
    void close() noexcept;

    bool valid() const noexcept;

    void wait();                   // semaphore - 1
    bool wait(const int64_t& ms);  // semaphore - 1 , timeout ms
    void release();                // semaphore + 1

   private:
    bool named_ = false;
    SemaphoreHandle* sh_ = nullptr;
};
}  // namespace veigar
#endif
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
    Mutex();
    ~Mutex();

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
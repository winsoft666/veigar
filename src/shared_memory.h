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
#ifndef VEIGAR_SHARED_MEMORY_H_
#define VEIGAR_SHARED_MEMORY_H_
#pragma once

#include "os_platform.h"
#include <inttypes.h>
#include <cstdint>
#include <string>

#ifdef VEIGAR_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif  // VEIGAR_OS_WINDOWS

namespace veigar {
class SharedMemory {
   public:
    // path should only contain alpha-numeric characters, and is normalized on linux/macOS.
    explicit SharedMemory(const std::string& path, int64_t size, bool create) noexcept;

    // open an existing shared memory for reading/writing
    inline bool open() noexcept {
        if (valid()) {
            return true;
        }
        return createOrOpen();
    }

    bool valid() const noexcept;

    void close() noexcept;

    inline int64_t size() const noexcept {
        return size_;
    }

    inline std::string path() const noexcept {
        return path_;
    }

    inline uint8_t* data() noexcept {
        return data_;
    }

    ~SharedMemory();

   private:
    bool createOrOpen() noexcept;

   private:
    bool create_ = false;
    std::string path_;
    uint8_t* data_ = nullptr;
    int64_t size_ = 0;
#ifdef VEIGAR_OS_WINDOWS
    HANDLE handle_ = NULL;
#else
    int fd_ = -1;
#endif
};
}  // namespace veigar
#endif
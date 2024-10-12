/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
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
    explicit SharedMemory(const std::string& path, int64_t size) noexcept;

    bool create();
    bool open();
    bool valid() const;
    void close();

    inline int64_t size() const {
        return size_;
    }

    inline std::string path() const {
        return path_;
    }

    inline uint8_t* data() {
        return data_;
    }

    ~SharedMemory() noexcept = default;
   private:
    std::string path_;
    uint8_t* data_ = nullptr;
    int64_t size_ = 0;
#ifdef VEIGAR_OS_WINDOWS
    HANDLE handle_ = NULL;
#else
    int fd_ = -1;
    bool shmCreator_ = false;
#endif
};
}  // namespace veigar
#endif
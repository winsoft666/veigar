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
#include "shared_memory.h"
#include "log.h"
#ifdef VEIGAR_OS_WINDOWS
#include <io.h>  // CreateFileMappingA, OpenFileMappingA, etc.
#include <assert.h>
#else                  // !VEIGAR_OS_WINDOWS
#include <fcntl.h>     // for O_* constants
#include <sys/mman.h>  // mmap, munmap
#include <sys/stat.h>  // for mode constants
#include <unistd.h>    // unlink

#ifdef VEIGAR_OS_MACOS
#include <errno.h>
#endif  // VEIGAR_OS_MACOS

#include <stdexcept>
#endif  // VEIGAR_OS_WINDOWS

namespace veigar {
#ifdef VEIGAR_OS_WINDOWS
SharedMemory::SharedMemory(const std::string& path, int64_t size, bool create) noexcept :
    path_(path),
    size_(size),
    create_(create) {
}

bool SharedMemory::createOrOpen() noexcept {
    if (path_.empty()) {
        return false;
    }

    if (create_) {
        DWORD sizeLowOrder = static_cast<DWORD>(size_);
        handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE,
                                     NULL,
                                     PAGE_READWRITE,
                                     0,
                                     sizeLowOrder,
                                     path_.c_str());

        if (!handle_) {
            veigar::log("Veigar: Error: Create file mapping failed, name: %s, size: %d, gle: %d.\n",
                        path_.c_str(), sizeLowOrder, GetLastError());
            return false;
        }
    }
    else {
        handle_ = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE,
                                   FALSE,
                                   path_.c_str());

        if (!handle_) {
            veigar::log("Veigar: Error: Open file mapping failed, name: %s, gle: %d.\n", path_.c_str(), GetLastError());
            return false;
        }
    }

    DWORD access = create_ ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ | FILE_MAP_WRITE;

    data_ = static_cast<uint8_t*>(MapViewOfFile(handle_, access, 0, 0, 0));

    if (!data_) {
        veigar::log("Veigar: Error: Map file view failed, name: %s, gle: %d.\n", path_.c_str(), GetLastError());
        if (handle_) {
            CloseHandle(handle_);
            handle_ = NULL;
        }
        return false;
    }

    return true;
}

bool SharedMemory::valid() const noexcept {
    return !!handle_;
}

void SharedMemory::close() noexcept {
    if (data_) {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }

    if (handle_) {
        CloseHandle(handle_);
        handle_ = NULL;
    }
}

SharedMemory::~SharedMemory() {
    close();
}
#else  // !VEIGAR_OS_WINDOWS

SharedMemory::SharedMemory(const std::string& path, int64_t size, bool create) noexcept :
    size_(size),
    create_(create) {
    // For portable use, a shared memory object should be identified by a name of the form / somename;
    path_ = "/" + path;
}

bool SharedMemory::createOrOpen() noexcept {
    if (path_.empty()) {
        return false;
    }

    int flags = create_ ? (O_CREAT | O_RDWR) : O_RDWR;

    fd_ = shm_open(path_.c_str(), flags, 0666);
    if (fd_ < 0) {
        int err = errno;
        veigar::log("Veigar: Error: %s shared memory failed, err: %d.\n", create_ ? "Create" : "Open", err);
        return false;
    }

    if (create_) {
        // this is the only way to specify the size of a newly-created POSIX shared memory object
        int ret = ftruncate(fd_, size_);
        if (ret != 0) {
            int err = errno;
            veigar::log("Veigar: Error: ftruncate shm failed, size: %" PRId64 ", err: %d.\n", size_, err);
            ::close(fd_);
            fd_ = -1;
            if (create_) {
                shm_unlink(path_.c_str());
            }
            return false;
        }
    }

    void* memory = mmap(nullptr,                 // addr
                        size_,                   // length
                        PROT_READ | PROT_WRITE,  // prot
                        MAP_SHARED,              // flags
                        fd_,                     // fd
                        0                        // offset
    );

    if (memory == MAP_FAILED) {
        int err = errno;
        veigar::log("Veigar: Error: mmap shm failed, size: %" PRId64 ", err: %d.\n", size_, err);

        ::close(fd_);
        fd_ = -1;
        if (create_) {
            shm_unlink(path_.c_str());
        }
        return false;
    }

    data_ = static_cast<uint8_t*>(memory);

    if (!data_) {
        ::close(fd_);
        fd_ = -1;
        if (create_) {
            shm_unlink(path_.c_str());
        }
        return false;
    }

    //veigar::log("Veigar: %s shared memory success, fd: %d.\n", create_ ? "Create" : "Open", fd_);
    return true;
}

bool SharedMemory::valid() const noexcept {
    return fd_ != -1;
}

void SharedMemory::close() noexcept {
    if (fd_ != -1) {
        //veigar::log("Veigar: Close fd: %d.\n", fd_);
        if (data_) {
            munmap(data_, size_);
            data_ = nullptr;
        }

        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }

        if (create_) {
            if (!path_.empty()) {
                shm_unlink(path_.c_str());
            }
        }
    }
}

SharedMemory::~SharedMemory() {
    close();
}

#endif  //VEIGAR_OS_WINDOWS
}  // namespace veigar
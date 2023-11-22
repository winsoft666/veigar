#ifndef VEIGAR_NAME_MANAGER_H_
#define VEIGAR_NAME_MANAGER_H_
#pragma once

#include "boost_itp.h"
#include <string>

namespace veigar {

class NameManager {
   public:
    typedef itp::allocator<char, itp::managed_shared_memory::segment_manager> CharAllocator;
    typedef itp::basic_string<char, std::char_traits<char>, CharAllocator> String;
    typedef itp::allocator<String, itp::managed_shared_memory::segment_manager> StringAllocator;
    typedef itp::deque<String, StringAllocator> StringDeque;

    NameManager() noexcept;
    ~NameManager() noexcept;

    bool init() noexcept;

    bool isInit() const noexcept;

    void uninit() noexcept;

    bool isExist(const std::string& name) noexcept;

    bool reserveName(const std::string& name) noexcept;

    bool releaseName(const std::string& name) noexcept;

   private:
    bool init_ = false;
    itp::managed_shared_memory sharedMemory_;
    StringDeque* names_ = nullptr;
};
}  // namespace veigar

#endif  // !VEIGAR_NAME_MANAGER_H_
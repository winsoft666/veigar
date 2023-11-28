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
#ifndef VEIGAR_SHARED_MEMEORY_H_
#define VEIGAR_SHARED_MEMEORY_H_
#pragma once

#include "boost_itp.h"
#include <vector>

namespace veigar {
typedef itp::managed_shared_memory managed_shared_memory;
typedef itp::managed_shared_memory::segment_manager segment_manager;
typedef itp::shared_memory_object shared_memory_object;

typedef itp::managed_shared_memory::allocator<char>::type CharAllocator;
typedef itp::basic_string<char, std::char_traits<char>, CharAllocator> String;

typedef managed_shared_memory::allocator<uint8_t>::type ByteAllocator;
typedef itp::vector<uint8_t, ByteAllocator> ByteVector;

typedef itp::interprocess_mutex Mutex;

class Message {
   public:
    Message(managed_shared_memory* msm);

    Message& operator=(const Message& other) {
        return copy(other);
    }

    ~Message();

    Message& copy(const Message& other);

    bool isEmpty() const;

    void clear();

    std::vector<uint8_t> getArg() const;

    void setArg(const std::vector<uint8_t>& argument);

   protected:
    ByteVector arg_;
    managed_shared_memory* msm_ = nullptr;
};

typedef itp::allocator<class Message, segment_manager> MessageAllocator;
typedef itp::deque<class Message, MessageAllocator> MessageDeque;

class MessageQueue {
   public:
    bool isInit() const noexcept;

    bool init(const std::string& segmentName, bool clearQueue, bool openOnly, unsigned int bufferSize) noexcept;
    void uninit() noexcept;

    bool pushBack(const std::vector<uint8_t>& buf, unsigned int timeout) noexcept;

    bool popFront(std::vector<uint8_t>& buf, unsigned int timeout) noexcept;

    bool clear(unsigned int timeout) noexcept;

    Message createMessage();

   protected:
    bool lock(unsigned int timeout);
    void unlock();

    bool isInit_ = false;
    std::string segmentName_;
    MessageDeque* messages_ = nullptr;
    Mutex* processMutex_ = nullptr;
    managed_shared_memory msm_;
};
}  // namespace veigar
#endif
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
#ifndef VEIGAR_MESSAGE_QUEUE_
#define VEIGAR_MESSAGE_QUEUE_
#pragma once

#include <memory>
#include <inttypes.h>
#include "shared_memory.h"
#include "mutex.h"
#include "semaphore.h"

namespace veigar {
class MessageQueue {
   public:
    MessageQueue(bool discardOldMsg, int32_t msgMaxNumber, int32_t msgExpectedMaxSize);
    ~MessageQueue();

    bool create(const std::string& path) noexcept;
    bool open(const std::string& path) noexcept;

    bool pushBack(uint32_t timeoutMS, const void* data, int64_t dataSize) noexcept;
    bool popFront(uint32_t timeoutMS, void* buf, int64_t bufSize, int64_t& written) noexcept;

    bool wait(int64_t ms) noexcept;

    bool isDiscardOldMsg() const noexcept;

    void close() noexcept;

    void notifyRead() noexcept;

   private:
    bool discardOldMsg_ = false;
    int32_t msgMaxNumber_ = 0;
    int32_t msgExpectedMaxSize_ = 0;
    std::shared_ptr<SharedMemory> shm_ = nullptr;
    std::shared_ptr<Mutex> rwLock_ = nullptr;
    std::shared_ptr<Semaphore> readSmp_ = nullptr;
};
}  // namespace veigar
#endif
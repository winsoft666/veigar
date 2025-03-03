/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_MESSAGE_QUEUE_
#define VEIGAR_MESSAGE_QUEUE_
#pragma once

#include <memory>
#include <inttypes.h>
#include "shared_memory.h"
#include "semaphore.h"

namespace veigar {
class MessageQueue {
   public:
    MessageQueue(int32_t msgMaxNumber, int32_t msgExpectedMaxSize) noexcept;
    ~MessageQueue() = default;

    bool create(const std::string& path);
    bool open(const std::string& path);
    void close();

    // Read/Write locker for multi processes.
    bool processRWLock(uint32_t timeoutMS);
    void processRWUnlock();

    // Need protect by process rw-lock
    bool pushBack(const void* data, int64_t dataSize);

    // Need protect by process rw-lock
    bool popFront(void* buf, int64_t bufSize, int64_t& written);

    // Need protect by process rw-lock
    int64_t msgNumber() const;

    // Need protect by process rw-lock
    bool checkSpaceSufficient(int64_t dataSize, bool& waitable) const;

    bool waitForRead(int64_t ms);

    void notifyRead();

   private:
    int32_t msgMaxNumber_ = 0;
    int32_t msgExpectedMaxSize_ = 0;
    std::shared_ptr<SharedMemory> shm_ = nullptr;
    std::shared_ptr<Semaphore> rwLock_ = nullptr;
    std::shared_ptr<Semaphore> readSmp_ = nullptr;
};
}  // namespace veigar
#endif
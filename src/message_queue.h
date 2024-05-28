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
#include "mutex.h"
#include "semaphore.h"

namespace veigar {
class MessageQueue {
   public:
    MessageQueue(bool discardOldMsg, int32_t msgMaxNumber, int32_t msgExpectedMaxSize) noexcept;
    ~MessageQueue() noexcept = default;

    bool create(const std::string& path);
    bool open(const std::string& path);

    bool rwLock(uint32_t timeoutMS);
    void rwUnlock();

    // Need protect by rw-lock
    bool pushBack(const void* data, int64_t dataSize);
    bool popFront(void* buf, int64_t bufSize, int64_t& written);
    bool checkSpaceSufficient(int64_t dataSize) const;

    bool wait(int64_t ms);

    bool isDiscardOldMsg() const;

    void close();

    void notifyRead();

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
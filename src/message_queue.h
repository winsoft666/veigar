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
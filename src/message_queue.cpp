/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "message_queue.h"
#include "log.h"
#include <assert.h>

namespace veigar {
MessageQueue::MessageQueue(int32_t msgMaxNumber, int32_t msgExpectedMaxSize) noexcept :
    msgMaxNumber_(msgMaxNumber),
    msgExpectedMaxSize_(msgExpectedMaxSize) {
}

bool MessageQueue::create(const std::string& path) {
    bool result = false;

    do {
        assert(!shm_ && !rwLock_ && !readSmp_);
        close();

        if (msgMaxNumber_ <= 0) {
            break;
        }

        int64_t shmSize = sizeof(int64_t) * (msgMaxNumber_ + 3) + msgMaxNumber_ * msgExpectedMaxSize_;

        std::string shmName = path + "_shm";
        shm_ = std::make_shared<SharedMemory>(shmName, shmSize);
        if (!shm_->create()) {
            break;
        }

        std::string rwLockName = path + "_rwlock";
        rwLock_ = std::make_shared<Semaphore>();
        if (!rwLock_->open(rwLockName.c_str(), 1, 1)) {
            break;
        }

        std::string readSmpName = path + "_readsmp";
        readSmp_ = std::make_shared<Semaphore>();
        if (!readSmp_->open(readSmpName, 0)) {
            break;
        }

        uint8_t* data = shm_->data();
        memset(data, 0, (size_t)shmSize);
        int64_t* pDataSize = (int64_t*)data;
        *pDataSize = shmSize;

        result = true;
    } while (false);

    if (!result) {
        if (shm_) {
            if (shm_->valid())
                shm_->close();
            shm_.reset();
        }

        if (rwLock_) {
            if (rwLock_->valid())
                rwLock_->close();
            rwLock_.reset();
        }

        if (readSmp_) {
            if (readSmp_->valid())
                readSmp_->close();
            readSmp_.reset();
        }
    }

    return result;
}

bool MessageQueue::open(const std::string& path) {
    bool result = false;
    do {
        assert(!shm_ && !rwLock_ && !readSmp_);
        close();

        std::string shmName = path + "_shm";
        int64_t shmSize = sizeof(int64_t) * (msgMaxNumber_ + 3) + msgMaxNumber_ * msgExpectedMaxSize_;

        shm_ = std::make_shared<SharedMemory>(shmName, shmSize);
        if (!shm_->open()) {
            break;
        }

        std::string rwLockName = path + "_rwlock";
        if (!Semaphore::IsExist(rwLockName)) {
            break;
        }

        rwLock_ = std::make_shared<Semaphore>();
        if (!rwLock_->open(rwLockName.c_str())) {
            break;
        }

        std::string readSmpName = path + "_readsmp";
        if (!Semaphore::IsExist(readSmpName)) {
            break;
        }

        readSmp_ = std::make_shared<Semaphore>();
        if (!readSmp_->open(readSmpName)) {
            break;
        }

        result = true;
    } while (false);

    if (!result) {
        if (shm_) {
            if (shm_->valid())
                shm_->close();
            shm_.reset();
        }

        if (rwLock_) {
            if (rwLock_->valid())
                rwLock_->close();
            rwLock_.reset();
        }

        if (readSmp_) {
            if (readSmp_->valid())
                readSmp_->close();
            readSmp_.reset();
        }
    }

    return result;
}

bool MessageQueue::rwLock(uint32_t timeoutMS) {
    if (!rwLock_)
        return false;
    return rwLock_->wait(timeoutMS);
}

void MessageQueue::rwUnlock() {
    rwLock_->release();
}

/*
| Shm Total Size | Msg Number | Front Free Size | Msg 0 Size | Msg 1 Size | ... | Msg 0 Data | Msg 1 Data | ... |
|      8         |      8     |       8         |    8       |    8       |     | Msg 0 Size | Msg 1 Size | ... |
*/
bool MessageQueue::pushBack(const void* data, int64_t dataSize) {
    bool ret = false;

    assert(data);
    assert(dataSize > 0);
    if (!data || dataSize <= 0) {
        return false;
    }

    assert(msgMaxNumber_ > 0);
    if (msgMaxNumber_ * msgExpectedMaxSize_ < dataSize) {
        veigar::log("Veigar: Error: The data size has exceeded the total size of the message queue. Please adjust the parameters of the message queue.\n");
        return false;
    }

    if (dataSize > msgExpectedMaxSize_) {
        veigar::log("Veigar: Warning: Message size(%" PRId64 ") greater than expected(%d). It's best to adjust the parameters of the message queue.\n", dataSize, msgExpectedMaxSize_);
    }

    do {
        uint8_t* const shmData = shm_->data();
        assert(shmData);
        if (!shmData) {
            break;
        }

        int64_t* const p64 = (int64_t*)shmData;
        if (!p64) {
            break;
        }

        int64_t* const pShmSize = p64;
        int64_t* const pCurMsgNumber = p64 + 1;
        int64_t* const pFrontFree = p64 + 2;
        int64_t* const pFirstMsgDataSize = p64 + 3;

        int64_t msgSizeHeaderTotalSize = msgMaxNumber_ * sizeof(int64_t);
        uint8_t* const pFirstMsgData = shmData + msgSizeHeaderTotalSize + sizeof(int64_t) * 3;

        int64_t msgDataTotalSize = 0L;
        if (*pCurMsgNumber > 0) {
            for (int64_t i = 0; i < *pCurMsgNumber; i++) {
                msgDataTotalSize += *(pFirstMsgDataSize + i);
            }
        }

        int64_t totalFree = *pShmSize - msgSizeHeaderTotalSize - sizeof(int64_t) * 3 - msgDataTotalSize;
        int64_t tailFree = totalFree - *pFrontFree;
        if (totalFree < dataSize || *pCurMsgNumber == msgMaxNumber_) {
            veigar::log("Veigar: Warning: Message queue is full. Please adjust the parameters of the message queue.\n");
            break;
        }

        uint8_t* pCopyBegin = nullptr;
        if (tailFree >= dataSize) {
            *pCurMsgNumber += 1;

            // record data size
            *(pFirstMsgDataSize + (*pCurMsgNumber - 1)) = dataSize;

            pCopyBegin = pFirstMsgData + msgDataTotalSize + *pFrontFree;
        }
        else {
            *pCurMsgNumber += 1;

            // record data size
            *(pFirstMsgDataSize + (*pCurMsgNumber - 1)) = dataSize;

            // move old data to offset zero
            uint8_t* pOldData = pFirstMsgData + *pFrontFree;
            memcpy(pFirstMsgData, pOldData, (size_t)msgDataTotalSize);

            // set front free to 0
            *pFrontFree = 0;

            pCopyBegin = pFirstMsgData + msgDataTotalSize;
        }

        assert(pCopyBegin);
        memcpy(pCopyBegin, data, (size_t)dataSize);

        ret = true;
    } while (false);

    if (ret) {
        readSmp_->release();
    }

    return ret;
}

bool MessageQueue::popFront(void* buf, int64_t bufSize, int64_t& written) {
    bool ret = false;

    do {
        written = 0;

        uint8_t* shmData = shm_->data();
        assert(shmData);
        if (!shmData) {
            break;
        }

        int64_t* const p64 = (int64_t*)shmData;
        if (!p64) {
            break;
        }

        int64_t* const pShmSize = p64;
        int64_t* const pCurMsgNumber = p64 + 1;
        if (*pCurMsgNumber <= 0) {
            break;
        }
        int64_t* const pFrontFree = p64 + 2;
        int64_t* const pFirstMsgDataSize = p64 + 3;

        if (*pFirstMsgDataSize > bufSize || !buf) {
            written = *pFirstMsgDataSize;
            break;
        }

        int64_t msgSizeHeaderTotalSize = msgMaxNumber_ * sizeof(int64_t);
        uint8_t* const pFirstMsgData = shmData + sizeof(int64_t) * 3 + msgSizeHeaderTotalSize;

        int64_t msgDataTotalSize = 0L;
        for (int64_t i = 0; i < *pCurMsgNumber; i++) {
            msgDataTotalSize += *(pFirstMsgDataSize + i);
        }

        // copy data
        uint8_t* pCopyBegin = pFirstMsgData + *pFrontFree;
        written = *pFirstMsgDataSize;

        memcpy(buf, pCopyBegin, (size_t)written);

        // set msg number
        *pCurMsgNumber -= 1;

        // set front free
        *pFrontFree += *pFirstMsgDataSize;

        // pop a element from data size list
        memcpy(pFirstMsgDataSize, pFirstMsgDataSize + 1, (size_t)(sizeof(int64_t) * (*pCurMsgNumber)));

        ret = true;
    } while (false);

    return ret;
}

int64_t MessageQueue::msgNumber() const {
    uint8_t* shmData = shm_->data();
    assert(shmData);
    if (!shmData) {
        return -1;
    }

    int64_t* const p64 = (int64_t*)shmData;
    if (!p64) {
        return -1;
    }

    int64_t* const pShmSize = p64;
    int64_t* const pCurMsgNumber = p64 + 1;

    return *pCurMsgNumber;
}

bool MessageQueue::wait(int64_t ms) {
    if (readSmp_) {
        return readSmp_->wait(ms);
    }
    return false;
}

bool MessageQueue::checkSpaceSufficient(int64_t dataSize, bool& waitable) const {
    assert(msgMaxNumber_ > 0);
    if (msgMaxNumber_ * msgExpectedMaxSize_ < dataSize) {
        waitable = false;
        veigar::log("Veigar: Error: The data size has exceeded the total size of the message queue. Please adjust the parameters of the message queue.\n");
        return false;
    }

    bool result = false;
    waitable = true;

    do {
        uint8_t* const shmData = shm_->data();
        assert(shmData);
        if (!shmData) {
            break;
        }

        int64_t* const p64 = (int64_t*)shmData;
        if (!p64) {
            break;
        }

        int64_t* const pShmSize = p64;
        int64_t* const pCurMsgNumber = p64 + 1;
        if (*pCurMsgNumber >= msgMaxNumber_) {
            break;
        }
        int64_t* const pFrontFree = p64 + 2;
        int64_t* const pFirstMsgDataSize = p64 + 3;

        int64_t msgSizeHeaderTotalSize = msgMaxNumber_ * sizeof(int64_t);
        uint8_t* const pFirstMsgData = shmData + msgSizeHeaderTotalSize + sizeof(int64_t) * 3;

        int64_t msgDataTotalSize = 0L;
        if (*pCurMsgNumber > 0) {
            for (int64_t i = 0; i < *pCurMsgNumber; i++) {
                msgDataTotalSize += *(pFirstMsgDataSize + i);
            }
        }

        int64_t totalFree = *pShmSize - msgSizeHeaderTotalSize - sizeof(int64_t) * 3 - msgDataTotalSize;
        if (totalFree < dataSize) {
            break;
        }

        result = true;
    } while (false);

    return result;
}

void MessageQueue::close() {
    if (shm_) {
        if (shm_->valid())
            shm_->close();
        shm_.reset();
    }

    if (rwLock_) {
        if (rwLock_->valid())
            rwLock_->close();
        rwLock_.reset();
    }

    if (readSmp_) {
        if (readSmp_->valid())
            readSmp_->close();
        readSmp_.reset();
    }
}

void MessageQueue::notifyRead() {
    if (readSmp_) {
        readSmp_->release();
    }
}
}  // namespace veigar
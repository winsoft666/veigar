#include "message_queue.h"
#include "log.h"
#include <assert.h>

namespace veigar {
MessageQueue::MessageQueue(bool discardOldMsg, int32_t msgMaxNumber, int32_t msgExpectedMaxSize) :
    discardOldMsg_(discardOldMsg),
    msgMaxNumber_(msgMaxNumber),
    msgExpectedMaxSize_(msgExpectedMaxSize) {
}

MessageQueue::~MessageQueue() {
    close();
}

bool MessageQueue::create(const std::string& path) noexcept {
    bool result = false;

    do {
        if (msgMaxNumber_ <= 0) {
            break;
        }

        int64_t shmSize = sizeof(int64_t) * (msgMaxNumber_ + 3) + msgMaxNumber_ * msgExpectedMaxSize_;

        std::string shmName = path + "_shm";
        shm_ = std::make_shared<SharedMemory>(shmName, shmSize, true);
        if (!shm_->open()) {
            veigar::log("Veigar: Create shm(%s) failed.\n", shmName.c_str());
            break;
        }

        std::string rwLockName = path + "_rwlock";
        rwLock_ = std::make_shared<Mutex>();
        if (!rwLock_->open(rwLockName.c_str())) {
            veigar::log("Veigar: Create mutex(%s) failed.\n", rwLockName.c_str());
            break;
        }

        std::string readSmpName = path + "_readsmp";
        readSmp_ = std::make_shared<Semaphore>();
        if (!readSmp_->open(readSmpName, 0)) {
            veigar::log("Veigar: Create semaphore(%s) failed.\n", readSmpName.c_str());
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

bool MessageQueue::open(const std::string& path) noexcept {
    bool result = false;
    do {
        std::string shmName = path + "_shm";
        int64_t shmSize = sizeof(int64_t) * (msgMaxNumber_ + 3) + msgMaxNumber_ * msgExpectedMaxSize_;

        shm_ = std::make_shared<SharedMemory>(shmName, shmSize, false);
        if (!shm_->open()) {
            veigar::log("Veigar: Open shm(%s) failed.\n", shmName.c_str());
            break;
        }

        std::string rwLockName = path + "_rwlock";
        rwLock_ = std::make_shared<Mutex>();
        if (!rwLock_->open(rwLockName.c_str())) {
            veigar::log("Veigar: Open mutex(%s) failed.\n", rwLockName.c_str());
            break;
        }

        std::string readSmpName = path + "_readsmp";
        readSmp_ = std::make_shared<Semaphore>();
        if (!readSmp_->open(readSmpName, 0)) {
            veigar::log("Veigar: Open semaphore(%s) failed.\n", readSmpName.c_str());
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

/*
| Shm Total Size | Msg Number | Front Free Size | Msg 0 Size | Msg 1 Size | ... | Msg 0 Data | Msg 1 Data | ... |
|      8           |      8       |       8           |    8         |    8        |      | Msg 0 Size | Msg 1 Size | ... |
*/
bool MessageQueue::pushBack(const void* data, int64_t dataSize) noexcept {
    bool ret = false;

    if (!data || dataSize <= 0) {
        return false;
    }

    assert(msgMaxNumber_ > 0);
    if (msgMaxNumber_ * msgExpectedMaxSize_ < dataSize) {
        return false;
    }

    if (dataSize > msgExpectedMaxSize_) {
        veigar::log("Veigar: Warning: Message size(%" PRId64 ") greater than expected(%d).\n", dataSize, msgExpectedMaxSize_);
    }

    rwLock_->lock(-1);
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
        if ((totalFree < dataSize || *pCurMsgNumber == msgMaxNumber_) && !discardOldMsg_) {
            break;
        }

        uint8_t* pCopyBegin = nullptr;
        if (*pCurMsgNumber == msgMaxNumber_ || totalFree < dataSize) {
            // discard old message data
            int64_t shortfall = dataSize - totalFree;
            int64_t discardMsgSize = 0L;
            int64_t discardMsgNum = 0L;
            uint8_t* msgDataEnd = nullptr;
            for (int64_t i = 0; i < *pCurMsgNumber; i++) {
                discardMsgNum++;
                discardMsgSize += *(pFirstMsgDataSize + i);
                if (discardMsgSize >= shortfall) {
                    break;
                }
            }

            // copy remaining data to offset 0, discard the first discardMsgSize bytes of data.
            memcpy(pFirstMsgData, pFirstMsgData + *pFrontFree + discardMsgSize, (size_t)(msgDataTotalSize - discardMsgSize));

            // set front free to 0
            *pFrontFree = 0L;

            // pop a element from data size list
            memcpy(pFirstMsgDataSize, pFirstMsgDataSize + discardMsgNum, (size_t)(sizeof(int64_t) * (msgMaxNumber_ - discardMsgNum)));

            *pCurMsgNumber = *pCurMsgNumber - discardMsgNum + 1;

            // record data size
            *(pFirstMsgDataSize + (*pCurMsgNumber - 1)) = dataSize;

            // append data
            pCopyBegin = pFirstMsgData + msgDataTotalSize - discardMsgSize;
        }
        else if (tailFree >= dataSize) {
            *pCurMsgNumber += 1;

            // record data size
            *(pFirstMsgDataSize + (*pCurMsgNumber - 1)) = dataSize;

            pCopyBegin = pFirstMsgData + msgDataTotalSize + *pFrontFree;
        }
        else if (totalFree >= dataSize) {
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
        else {
            assert(false);
        }

        assert(pCopyBegin);
        memcpy(pCopyBegin, data, (size_t)dataSize);

        ret = true;
    } while (false);

    rwLock_->unlock();

    if (ret) {
        readSmp_->release();
    }

    return ret;
}

bool MessageQueue::popFront(void* buf, int64_t bufSize, int64_t& written) noexcept {
    bool ret = false;

    if (!buf || bufSize <= 0) {
        return false;
    }

    rwLock_->lock(-1);

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

        if (*pFirstMsgDataSize > bufSize) {
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
        memcpy(pFirstMsgDataSize, pFirstMsgDataSize + 1,  (size_t)(sizeof(int64_t) * (*pCurMsgNumber)));

        ret = true;
    } while (false);

    rwLock_->unlock();

    return ret;
}

bool MessageQueue::wait(int64_t ms) noexcept {
    if (readSmp_) {
        return readSmp_->wait(ms);
    }
    return false;
}

bool MessageQueue::isDiscardOldMsg() const noexcept {
    return discardOldMsg_;
}

void MessageQueue::close() noexcept {
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

void MessageQueue::notifyRead() noexcept {
    if (readSmp_) {
        readSmp_->release();
    }
}

}  // namespace veigar
#include "shared_memory.h"
#include "log.h"
#include <chrono>

namespace veigar {
Message::Message(managed_shared_memory* msm) :
    arg_(ByteAllocator(msm->get_segment_manager())),
    msm_(msm) {
}

Message::~Message() {
}

Message& Message::copy(const Message& other) {
    arg_ = other.arg_;

    return *this;
}

bool Message::isEmpty() const {
    return arg_.size() == 0;
}

void Message::clear() {
    arg_.clear();
}

std::vector<uint8_t> Message::getArg() const {
    std::vector<uint8_t> ret;

    for (size_t i = 0; i < arg_.size(); i++) {
        ret.push_back(arg_[i]);
    }

    return ret;
}

void Message::setArg(const std::vector<uint8_t>& argument) {
    arg_.resize(argument.size());
    for (size_t i = 0; i < argument.size(); i++) {
        arg_[i] = argument[i];
    }
}

bool MessageQueue::isInit() const noexcept {
    return isInit_;
}

bool MessageQueue::init(const std::string& segmentName, bool create, unsigned int defaultBufferSize) noexcept {
    if (isInit_) {
        return false;
    }

    if (segmentName.empty()) {
        return false;
    }

    segmentName_ = segmentName;

    try {
        if (create) {
            itp::shared_memory_object::remove(segmentName_.c_str());
            msm_ = managed_shared_memory(itp::create_only, segmentName_.c_str(), defaultBufferSize);
        }
        else {
            msm_ = managed_shared_memory(itp::open_only, segmentName_.c_str());
        }

        mutex_ = msm_.find_or_construct<Mutex>("MessageDequeMutex")();
        messages_ = msm_.find_or_construct<MessageDeque>("MessageDeque")(MessageAllocator(msm_.get_segment_manager()));

        if (!messages_) {
            veigar::log("Veigar: find/construct MessageDeque failed on segment %s.\n", segmentName_.c_str());
            return false;
        }

        veigar::log("Veigar: create message queue(%s) success: %d.\n", segmentName_.c_str(), messages_->size());
    } catch (itp::interprocess_exception& exc) {
        veigar::log("Veigar: interprocess exception on init message queue(%s): %s.\n", segmentName_.c_str(), exc.what());
        return false;
    } catch (std::exception& exc) {
        veigar::log("Veigar: std exception on init message queue(%s): %s.\n", segmentName_.c_str(), exc.what());
        return false;
    }

    return true;
}

void MessageQueue::uninit() noexcept {
    if (!isInit_) {
        return;
    }
    messages_ = nullptr;
}

bool MessageQueue::pushBack(const std::vector<uint8_t>& buf, unsigned int timeout) {
    try {
        if (!lock(timeout)) {
            return false;
        }

        if (!messages_) {
            unlock();
            return false;
        }

        Message msg(&msm_);
        msg.setArg(buf);

        messages_->push_back(msg);

        unlock();
        return true;
    } catch (itp::interprocess_exception& exc) {
        veigar::log("Veigar: interprocess exception on push message to queue(%s): %s.\n", segmentName_.c_str(), exc.what());

    } catch (std::exception& exc) {
        veigar::log("Veigar: std exception on push message to queue(%s): %s.\n", segmentName_.c_str(), exc.what());
    }
    unlock();
    return false;
}

bool MessageQueue::popFront(Message* msg, unsigned int timeout) {
    if (!lock(timeout)) {
        return false;
    }

    if (!messages_ || messages_->size() == 0) {
        unlock();
        return false;
    }

    *msg = messages_->front();
    messages_->pop_front();

    unlock();
    return true;
}

Message MessageQueue::createMessage() {
    return Message(&msm_);
}

bool MessageQueue::lock(unsigned int timeout) {
    bool result = false;
    if (mutex_) {
        if (timeout > 0) {
            std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
            result = mutex_->timed_lock(deadline);
        }
        else {
            mutex_->lock();
            result = true;
        }
    }
    return result;
}

void MessageQueue::unlock() {
    if (mutex_) {
        mutex_->unlock();
    }
}

}  // namespace veigar
#include "semaphore.h"

namespace veigar {
Semaphore::Semaphore() {
}

Semaphore::~Semaphore() {
}

bool Semaphore::open(const std::string& name, int value /*= 0*/) noexcept {
#ifdef VEIGAR_OS_WINDOWS
    h_ = CreateSemaphoreA(NULL,
                          value,
                          100000,
                          name.c_str());
    return !!h_;
#else
    h_ = ::sem_open(name.c_str(), O_CREAT, 0666, static_cast<unsigned>(value));
    return h_ != SEM_FAILED;
#endif
}

void Semaphore::close() noexcept {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        CloseHandle(h_);
        h_ = nullptr;
#else
        if (::sem_close(h_) != 0) {
        }
        h_ = SEM_FAILED;
#endif
    }
}

bool Semaphore::valid() const noexcept {
#ifdef VEIGAR_OS_WINDOWS
    return !!h_;
#else
    return h_ != SEM_FAILED;
#endif
}

void Semaphore::wait() {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        WaitForSingleObject(h_, INFINITE);
#else
        sem_wait(h_);
#endif
    }
}

bool Semaphore::wait(const int64_t& ms) {
    if (!valid()) {
        return false;
    }

#ifdef VEIGAR_OS_WINDOWS
    DWORD result = WaitForSingleObject(h_, ms >= 0 ? (DWORD)ms : INFINITE);
    return (result == WAIT_OBJECT_0);
#else
    timeval tv_now;
    gettimeofday(&tv_now, NULL);

    timespec ts;
    ts.tv_sec = tv_now.tv_sec;
    ts.tv_nsec = tv_now.tv_usec * 1000;

    int ns = ts.tv_nsec + (ms % 1000) * 1000000;
    ts.tv_nsec = ns % 1000000000;
    ts.tv_sec += ns / 1000000000;
    ts.tv_sec += ms / 1000;

    if (sem_timedwait(h_, &ts) != 0) {
        return true;
    }
    return false;
#endif
}

void Semaphore::release() {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        ReleaseSemaphore(h_, 1, NULL);
#else
        sem_post(h_);
#endif
    }
}

}  // namespace veigar
#include "semaphore.h"

namespace veigar {
Semaphore::Semaphore() {
}

Semaphore::~Semaphore() {
    close();
}

bool Semaphore::open(const std::string& name, int value /*= 0*/) noexcept {
    named_ = !name.empty();
    sh_ = new SemaphoreHandle();
#ifdef VEIGAR_OS_WINDOWS
    HANDLE h = CreateSemaphoreA(NULL,
                                value,
                                100000,
                                name.empty() ? NULL : name.c_str());
    if (h) {
        sh_->h_ = h;
    }
    else {
        delete sh_;
        sh_ = nullptr;
    }
#else
    if (name.empty()) {
        int err = ::sem_init(&sh_->unnamed_, 0, 0);
        if (err == -1) {
            delete sh_;
            sh_ = nullptr;
        }
    }
    else {
        sem_t* sem = ::sem_open(name.c_str(), O_CREAT, 0666, static_cast<unsigned>(value));
        if (sem != SEM_FAILED) {
            sh_->named_ = sem;
        }
        else {
            delete sh_;
            sh_ = nullptr;
        }
    }
#endif
    return valid();
}

void Semaphore::close() noexcept {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        CloseHandle(sh_->h_);
        sh_->h_ = nullptr;
#else
        if (named_) {
            ::sem_close(sh_->named_);
            sh_->named_ = SEM_FAILED;
        }
        else {
            ::sem_destroy(&sh_->unnamed_);
        }
#endif

        delete sh_;
        sh_ = nullptr;
    }
}

bool Semaphore::valid() const noexcept {
    return !!sh_;
}

void Semaphore::wait() {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        WaitForSingleObject(sh_->h_, INFINITE);
#else
        if (named_) {
            sem_wait(sh_->named_);
        }
        else {
            sem_wait(&sh_->unnamed_);
        }
#endif
    }
}

bool Semaphore::wait(const int64_t& ms) {
    if (!valid()) {
        return false;
    }

#ifdef VEIGAR_OS_WINDOWS
    DWORD result = WaitForSingleObject(sh_->h_, ms >= 0 ? (DWORD)ms : INFINITE);
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

    if (named_) {
        if (sem_timedwait(sh_->named_, &ts) != 0) {
            return true;
        }
        return false;
    }
    else {
        if (sem_timedwait(&sh_->unnamed_, &ts) != 0) {
            return true;
        }
        return false;
    }
#endif
}

void Semaphore::release() {
    if (valid()) {
#ifdef VEIGAR_OS_WINDOWS
        ReleaseSemaphore(sh_->h_, 1, NULL);
#else
        if (named_) {
            sem_post(sh_->named_);
        }
        else {
            sem_post(&sh_->unnamed_);
        }
#endif
    }
}

}  // namespace veigar
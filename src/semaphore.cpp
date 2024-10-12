/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "semaphore.h"
#include "log.h"
#include <assert.h>
#include <limits>

namespace veigar {

bool Semaphore::IsExist(const std::string& name) {
#ifdef VEIGAR_OS_WINDOWS
    HANDLE h = OpenSemaphoreA(EVENT_MODIFY_STATE, FALSE, name.c_str());
    if (h) {
        CloseHandle(h);
    }
    return (h != NULL);
#else
    sem_t* sem = ::sem_open(name.c_str(), O_CREAT | O_EXCL);
    if (sem == SEM_FAILED) {
        return true;
    }
    ::sem_close(sem);
    return false;
#endif
}

bool Semaphore::open(const std::string& name, int value /*= 0*/, int maxValue /*= 2147483647*/) {
    assert(!valid());
    close();

    named_ = !name.empty();
    sh_ = new SemaphoreHandle();
#ifdef VEIGAR_OS_WINDOWS
    HANDLE h = CreateSemaphoreA(NULL, value, maxValue, name.empty() ? NULL : name.c_str());
    if (h) {
        sh_->h_ = h;
    }
    else {
        veigar::log("Veigar: Error: CreateSemaphoreA failed, name: %s, gle: %d.\n", name.c_str(), GetLastError());
        delete sh_;
        sh_ = nullptr;
    }
#else
    if (name.empty()) {
        // unnamed semaphore shared between multiple threads.
        int err = ::sem_init(&sh_->unnamed_, 0, 0);
        if (err == -1) {
            int err = errno;
            veigar::log("Veigar: Error: sem_init failed, name: %s, errno: %d.\n", name.c_str(), err);
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
            int err = errno;
            veigar::log("Veigar: Error: sem_open failed, name: %s, errno: %d.\n", name.c_str(), err);
            delete sh_;
            sh_ = nullptr;
        }
    }
#endif
    return valid();
}

void Semaphore::close() {
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

bool Semaphore::valid() const {
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
    timespec ts;

    if (ms > 0) {
        timespec ts_now;
        clock_gettime(CLOCK_REALTIME, &ts_now);

        ts.tv_sec = ts_now.tv_sec;
        ts.tv_nsec = ts_now.tv_nsec;

        ts.tv_sec += (ms / 1000);
        ts.tv_nsec += ((ms % 1000) * 1000000);
    }
    else {
        ts.tv_sec = std::numeric_limits<time_t>::max();
        ts.tv_nsec = 0;
    }

    if (named_) {
        if (sem_timedwait(sh_->named_, &ts) == 0) {
            return true;
        }
        return false;
    }
    else {
        if (sem_timedwait(&sh_->unnamed_, &ts) == 0) {
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
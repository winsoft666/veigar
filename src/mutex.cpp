/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mutex.h"
#include <assert.h>

namespace veigar {
#ifdef VEIGAR_OS_WINDOWS
bool Mutex::valid() const {
    return !!h_;
}

bool Mutex::open(const char* name) {
    close();
    h_ = new MutexHandle();
    h_->pMutex = ::CreateMutexA(NULL, FALSE, name);
    if (!h_->pMutex) {
        delete h_;
        h_ = nullptr;
    }
    return !!h_;
}

void Mutex::close() {
    if (!h_)
        return;

    ::CloseHandle(h_->pMutex);
    h_->pMutex = NULL;

    delete h_;
    h_ = nullptr;
}

bool Mutex::lock(int64_t ms) {
    assert(h_);
    if (!h_) {
        return false;
    }

    DWORD ret;
    DWORD dwMS = (ms == -1 ? INFINITE : (DWORD)ms);

    for (;;) {
        switch ((ret = ::WaitForSingleObject(h_->pMutex, dwMS))) {
            case WAIT_OBJECT_0:
                return true;
            case WAIT_TIMEOUT:
                return false;
            case WAIT_ABANDONED:
                if (!unlock()) {
                    return false;
                }
                break;  // loop again
            default:
                return false;
        }
    }
}

bool Mutex::tryLock() {
    assert(h_);
    if (!h_)
        return false;

    DWORD ret = ::WaitForSingleObject(h_->pMutex, 0);
    switch (ret) {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_TIMEOUT:
            return false;
        case WAIT_ABANDONED:
            unlock();

        default:
            return false;
    }
}

bool Mutex::unlock() {
    assert(h_);
    if (!h_) {
        return false;
    }
    if (!::ReleaseMutex(h_->pMutex)) {
        return false;
    }
    return true;
}
#else
bool Mutex::valid() const {
    return !!h_;
}

bool Mutex::open(const char* name) {
    close();
    h_ = new MutexHandle();
    int eno = ::pthread_mutex_init(&h_->pMutex, nullptr);
    if (eno != 0) {
        delete h_;
        h_ = nullptr;
        return false;
    }

    return true;
}

void Mutex::close() {
    if (h_) {
        int eno = ::pthread_mutex_destroy(&h_->pMutex);
        delete h_;
        h_ = nullptr;
    }
}

bool Mutex::lock(int64_t ms) {
    if (!h_)
        return false;

    for (;;) {
        int eno;
        if (ms < 0) {
            eno = ::pthread_mutex_lock(&h_->pMutex);
        }
        else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += ms / 1000;
            ts.tv_nsec = (ms % 1000) * 1000000;

            eno = ::pthread_mutex_timedlock(&h_->pMutex, &ts);
        }

        switch (eno) {
            case 0:
                return true;
            case ETIMEDOUT:
                return false;
            case EOWNERDEAD: {
                int eno2 = ::pthread_mutex_consistent(&h_->pMutex);
                if (eno2 != 0) {
                    return false;
                }
                int eno3 = ::pthread_mutex_unlock(&h_->pMutex);
                if (eno3 != 0) {
                    return false;
                }
            } break;  // loop again
            default:
                return false;
        }
    }
}

bool Mutex::tryLock() {
    if (!h_)
        return false;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    int eno = ::pthread_mutex_timedlock(&h_->pMutex, &ts);
    switch (eno) {
        case 0:
            return true;
        case ETIMEDOUT:
            return false;
        case EOWNERDEAD: {
            int eno2 = ::pthread_mutex_consistent(&h_->pMutex);
            if (eno2 != 0) {
                break;
            }
            int eno3 = ::pthread_mutex_unlock(&h_->pMutex);
            if (eno3 != 0) {
                break;
            }
        } break;
        default:
            break;
    }
    return false;
}

bool Mutex::unlock() {
    if (!h_)
        return false;

    int eno;
    if ((eno = ::pthread_mutex_unlock(&h_->pMutex)) != 0) {
        return false;
    }
    return true;
}

#endif
}  // namespace veigar
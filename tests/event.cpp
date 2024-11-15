/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "event.h"

#ifdef max
#undef max
#endif

Event::Event(bool isSet) noexcept :
    isSet_(isSet) {}

void Event::set() {
    std::unique_lock<std::mutex> ul(m_);
    isSet_ = true;
    settedCondVar_.notify_all();
}

void Event::cancel() {
    std::unique_lock<std::mutex> ul(m_);
    isCancelled_ = true;
    settedCondVar_.notify_all();
}

void Event::unCancel() {
    std::unique_lock<std::mutex> ul(m_);
    isCancelled_ = false;
    settedCondVar_.notify_all();
}

void Event::unset() {
    std::unique_lock<std::mutex> ul(m_);
    isSet_ = false;
    settedCondVar_.notify_all();
}

void Event::reset() {
    std::unique_lock<std::mutex> ul(m_);
    isSet_ = false;
    isCancelled_ = false;
    settedCondVar_.notify_all();
}

bool Event::isSet() {
    std::unique_lock<std::mutex> ul(m_);
    return isSet_;
}

bool Event::isCancelled() {
    std::unique_lock<std::mutex> ul(m_);
    return isCancelled_;
}

bool Event::wait(int64_t millseconds) {
    std::unique_lock<std::mutex> ul(m_);
    int64_t m = (millseconds >= 0 ? millseconds : std::chrono::duration_values<int64_t>::max());
    settedCondVar_.wait_for(ul, std::chrono::milliseconds(m), [this] { return (isSet_ || isCancelled_); });
    return (isSet_ || isCancelled_);
}
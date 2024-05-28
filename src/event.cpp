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

namespace veigar {
Event::Event(bool isSet) noexcept :
    is_set_(isSet) {}

void Event::set() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    is_set_ = true;
    setted_cond_var_.notify_all();
}

void Event::cancel() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    is_cancelld_ = true;
    setted_cond_var_.notify_all();
}

void Event::unCancel() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    is_cancelld_ = false;
    setted_cond_var_.notify_all();
}

void Event::unset() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    is_set_ = false;
    setted_cond_var_.notify_all();
}

void Event::reset() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    is_set_ = false;
    is_cancelld_ = false;
    setted_cond_var_.notify_all();
}

bool Event::isSet() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    return is_set_;
}

bool Event::isCancelled() {
    std::unique_lock<std::mutex> ul(set_mutex_);
    return is_cancelld_;
}

bool Event::wait(int64_t millseconds) {
    std::unique_lock<std::mutex> ul(set_mutex_);
    int64_t m = (millseconds >= 0 ? millseconds : std::chrono::duration_values<int64_t>::max());
    setted_cond_var_.wait_for(ul, std::chrono::milliseconds(m), [this] { return (is_set_ || is_cancelld_); });
    return is_set_;
}
}  // namespace veigar

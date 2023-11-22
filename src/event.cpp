/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023 winsoft666 <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
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
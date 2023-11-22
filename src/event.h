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

#ifndef ASHE_EVENT_HPP__
#define ASHE_EVENT_HPP__
#pragma once

#include <mutex>
#include <condition_variable>

namespace veigar {
class Event {
   public:
    Event(bool isSet = false) noexcept;

    ~Event() = default;

    void set();

    void unset();

    void cancel();

    void unCancel();

    // is equal unset() and unCancel().
    void reset();

    bool isSet();

    bool isCancelled();

    // is infinite when millseconds < 0.
    bool wait(int64_t millseconds = -1);

   protected:
    bool is_set_ = false;
    bool is_cancelld_ = false;
    std::mutex set_mutex_;
    std::condition_variable setted_cond_var_;
};
}  // namespace veigar
#endif  //!ASHE_EVENT_HPP__

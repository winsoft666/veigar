/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023-2024 winsoft666 <winsoft666@outlook.com>.
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
#ifndef VEIGAR_DETAIL_METER_H_
#define VEIGAR_DETAIL_METER_H_
#pragma once

#include <chrono>

namespace veigar {
namespace detail {
class TimeMeter {
   public:
    inline TimeMeter() {
        restart();
    }

    inline void restart() {
        lStartTime_ = std::chrono::high_resolution_clock::now();
    }

    // microseconds
    inline int64_t elapsed() const {
        auto now = std::chrono::high_resolution_clock::now();
        int64_t duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lStartTime_).count();
        return duration;
    }

   private:
    std::chrono::high_resolution_clock::time_point lStartTime_;
};
}  // namespace detail
}  // namespace veigar

#endif  //!VEIGAR_DETAIL_METER_H_
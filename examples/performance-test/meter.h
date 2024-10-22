/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_METER_H_
#define VEIGAR_DETAIL_METER_H_
#pragma once

#include <chrono>

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

#endif  //!VEIGAR_DETAIL_METER_H_
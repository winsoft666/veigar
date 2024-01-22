/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_TIME_UTIL_H_
#define VEIGAR_TIME_UTIL_H_
#pragma once

#include <stdint.h>

namespace veigar {
class TimeUtil {
   public:
    // The microseconds that since 1970-01-01 00:00:00(UTC)
    static int64_t GetCurrentTimestamp() noexcept;
};
}  // namespace veigar
#endif  // !VEIGAR_TIME_UTIL_H_
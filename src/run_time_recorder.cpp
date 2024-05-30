/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "run_time_recorder.h"
#include <inttypes.h>
#include "time_util.h"
#include "log.h"

namespace veigar {
RunTimeRecorder::RunTimeRecorder(const std::string& flag) :
    flag_(flag) {
    startTime_ = TimeUtil::GetCurrentTimestamp();
    log("[Veigar-RTR] %s Start at %" PRId64 ".\n", flag_.c_str(), startTime_);
}

RunTimeRecorder::~RunTimeRecorder() {
    if (!ended_) {
        end();
    }
}

void RunTimeRecorder::end() {
    int64_t now = TimeUtil::GetCurrentTimestamp();
    log("[Veigar-RTR] %s End at %" PRId64 ", used %" PRId64 ".\n", flag_.c_str(), now, now - startTime_);
    ended_ = true;
}
}  // namespace veigar
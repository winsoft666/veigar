/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DEBUG_HELPER_H_
#define VEIGAR_DEBUG_HELPER_H_
#pragma once

#include "veigar/config.h"
#include <string>

namespace veigar {
class RunTimeRecorder {
   public:
    RunTimeRecorder(const std::string& flag);
    ~RunTimeRecorder();

    void end();

   private:
    bool ended_ = false;
    int64_t startTime_ = 0;
    std::string flag_;
};
}  // namespace veigar

#ifdef VEIGAR_ENABLE_RUN_TIME_RECORD
#define RUN_TIME_RECORDER(flag) RunTimeRecorder __rtr__(flag)
#define RUN_TIME_RECORDER_END __rtr__.end()

#define RUN_TIME_RECORDER_EX(name, flag) RunTimeRecorder __##name##__(flag)
#define RUN_TIME_RECORDER_EX_END(name) __##name##__.end()
#else
#define RUN_TIME_RECORDER(flag)
#define RUN_TIME_RECORDER_END

#define RUN_TIME_RECORDER_EX(name, flag)
#define RUN_TIME_RECORDER_EX_END(name)
#endif

#endif  // !VEIGAR_DEBUG_HELPER_H_

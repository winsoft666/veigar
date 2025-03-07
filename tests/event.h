/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_EVENT_H__
#define VEIGAR_EVENT_H__
#pragma once

#include <mutex>
#include <condition_variable>

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
    bool isSet_ = false;
    bool isCancelled_ = false;
    std::mutex m_;
    std::condition_variable settedCondVar_;
};
#endif  //!VEIGAR_EVENT_H__

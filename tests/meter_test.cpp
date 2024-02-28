/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "catch.hpp"
#include "veigar/detail/meter.h"
#include <thread>

TEST_CASE("meter-now") {
    veigar::detail::TimeMeter tm;
    int64_t elapsed1 = tm.elapsed();
    int64_t elapsed2 = tm.elapsed();
    int64_t elapsed3 = tm.elapsed();
    int64_t elapsed4 = tm.elapsed();
    int64_t elapsed5 = tm.elapsed();

    REQUIRE(abs(elapsed2 - elapsed1) <= 3);
    REQUIRE(abs(elapsed3 - elapsed2) <= 3);
    REQUIRE(abs(elapsed4 - elapsed3) <= 3);
    REQUIRE(abs(elapsed5 - elapsed4) <= 3);
}

TEST_CASE("meter-thread") {
    std::vector<int> microseconds = {50000, 100000, 500000, 1000000};
    for (auto ms : microseconds) {
        std::thread t = std::thread([ms]() {
            veigar::detail::TimeMeter tm;
            std::this_thread::sleep_for(std::chrono::microseconds(ms));
            int64_t elapsed = tm.elapsed();

            int mistakeRange = (int)((float)ms / 5.f);
            REQUIRE(abs(elapsed - ms) <= mistakeRange);
        });
        t.join();
    }
}
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
#include <iostream>
#include <atomic>
#include "catch.hpp"
#include "../src/semaphore.h"
#include "thread_group.h"

TEST_CASE("semaphore-named") {
    veigar::Semaphore smp;
    REQUIRE(smp.open("43464FE463F14B458D5585DE378F5FFC"));
    int64_t ret = 0;
    ThreadGroup tg;
    tg.createThreads(4, [&smp, &ret](std::size_t id) {
        for (int i = 0; i < 100000; i++) {
            if (id != 0) {
                smp.release();
            }
            else {
                smp.wait(-1);
                ret++;
            }
        }
    });

    tg.joinAll();
    smp.close();

    REQUIRE(ret == 100000);
}

TEST_CASE("semaphore-unnamed") {
    veigar::Semaphore smp;
    REQUIRE(smp.open(""));
    int64_t ret = 0;
    ThreadGroup tg;
    tg.createThreads(4, [&smp, &ret](std::size_t id) {
        for (int i = 0; i < 100000; i++) {
            if (id != 0) {
                smp.release();
            }
            else {
                smp.wait(-1);
                ret++;
            }
        }
    });

    tg.joinAll();
    smp.close();

    REQUIRE(ret == 100000);
}
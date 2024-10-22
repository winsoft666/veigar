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
#include <list>
#include "../src/time_util.h"

TEST_CASE("semaphore-named") {
    veigar::Semaphore smp;
    REQUIRE(smp.open("semaphore-named-" + std::to_string(time(nullptr))));

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

TEST_CASE("semaphore-time-wait") {
    using namespace veigar;
    veigar::Semaphore rwLocker;
    REQUIRE(rwLocker.open("semaphore-time-wait-" + std::to_string(time(nullptr)), 0, 1));

    int64_t start = TimeUtil::GetCurrentTimestamp();
    REQUIRE(!rwLocker.wait(1000));
    int64_t end = TimeUtil::GetCurrentTimestamp();

    int64_t used = end - start;
    REQUIRE((used > 500000));

    rwLocker.close();
}

TEST_CASE("semaphore-rw-locker") {
    veigar::Semaphore rwLocker;
    REQUIRE(rwLocker.open("semaphore-rw-locker-" + std::to_string(time(nullptr)), 1, 1));

    std::list<std::string> list;
    ThreadGroup tg;
    int w = 0, r = 0;
    tg.createThreads(2, [&rwLocker, &w, &r, &list](std::size_t id) {
        for (int i = 0; i < 100000; i++) {
            CHECK(rwLocker.wait(1000));
            if (id % 2 == 0) {
                list.push_back("123456");
                w++;
            }
            else {
                if (list.size() > 0) {
                    std::string s = list.front();
                    CHECK(s == "123456");
                    list.pop_front();
                    r++;
                }
            }
            rwLocker.release();
        }
    });

    tg.joinAll();
    rwLocker.close();

    REQUIRE(w == 100000);
    REQUIRE(r > 0);
}
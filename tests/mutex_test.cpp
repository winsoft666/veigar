#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "catch.hpp"
#include "../src/mutex.h"
#include "thread_group.h"

TEST_CASE("mutex-open-close") {
    veigar::Mutex mtx;
    REQUIRE(mtx.open("DF396EB13C7546C0BBD12100E40F3B33"));
    mtx.close();
}

TEST_CASE("mutex") {
    veigar::Mutex mtx;
    REQUIRE(mtx.open("DF396EB13C7546C0BBD12100E40F3B33"));
    int64_t ret = 0;
    ThreadGroup tg;
    tg.createThreads(4, [&mtx, &ret](std::size_t id) {
        REQUIRE(mtx.lock(-1));
        for (int i = 0; i < 1000000; i++) {
            if (id % 2 == 0) {
                ret--;
            }
            else {
                ret++;
            }
        }
        REQUIRE(mtx.unlock());
    });

    tg.joinAll();
    mtx.close();

    REQUIRE(ret == 0);
}
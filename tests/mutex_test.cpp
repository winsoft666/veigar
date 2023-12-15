#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "catch.hpp"
#include "../src/mutex.h"
#include "thread_group.h"
#include "veigar/detail/meter.h"

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

TEST_CASE("named_mutex_timed_wait") {
    veigar::Mutex mtx;
    REQUIRE(mtx.open("DF396EB13C7546C0BBD12100E40F3B35"));

    std::thread t1 = std::thread([&mtx]() {
        REQUIRE(mtx.lock(0));

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        REQUIRE(mtx.unlock());
    });

    std::thread t2 = std::thread([&mtx]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        veigar::detail::TimeMeter tm;

        REQUIRE(!mtx.tryLock());

        REQUIRE(!mtx.lock(500)); // 500ms

        int64_t elapsed = tm.elapsed();  // microsecond
        REQUIRE(abs(elapsed - 500000L) <= 100000L);

        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        REQUIRE(mtx.tryLock());
        REQUIRE(mtx.lock(100));
        REQUIRE(mtx.unlock());
    });

    t1.join();
    t2.join();
    mtx.close();
}

TEST_CASE("unnamed_mutex_timed_wait") {
    veigar::Mutex mtx;
    REQUIRE(mtx.open("DF396EB13C7546C0BBD12100E40F3B35"));

    std::thread t1 = std::thread([&mtx]() {
        REQUIRE(mtx.lock(0));

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        REQUIRE(mtx.unlock());
    });

    std::thread t2 = std::thread([&mtx]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        veigar::detail::TimeMeter tm;

        REQUIRE(!mtx.tryLock());

        REQUIRE(!mtx.lock(500));  // 500ms

        int64_t elapsed = tm.elapsed();  // microsecond
        REQUIRE(abs(elapsed - 500000L) <= 100000L);

        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        REQUIRE(mtx.tryLock());
        REQUIRE(mtx.lock(100));
        REQUIRE(mtx.unlock());
    });

    t1.join();
    t2.join();
    mtx.close();
}
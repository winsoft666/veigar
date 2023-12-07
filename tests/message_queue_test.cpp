#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <map>
#include "catch.hpp"
#include "../src/message_queue.h"
#include <vector>
#include "thread_group.h"
#include <mutex>

TEST_CASE("mq-create-open") {
    std::string mqPath = "mq-create-open-641EC1C27FE244C9B";

    veigar::MessageQueue mq1(false, 6, 10);
    REQUIRE(mq1.create(mqPath));

    veigar::MessageQueue mq2(false, 6, 10);
    REQUIRE(mq2.open(mqPath));

    mq1.close();
    REQUIRE(!mq2.open(mqPath));

    mq2.close();
    REQUIRE(!mq2.open(mqPath));

    REQUIRE(mq1.create(mqPath));
    mq1.close();
}

TEST_CASE("mq-push-pop-no-discard") {
    std::string mqPath = "mq-create-open-641EC1C27FE244C9B";
    std::string data = "hello-123456";  // size = 12

    veigar::MessageQueue mq1(false, 3, 10);  // max size = 30
    REQUIRE(mq1.create(mqPath));

    REQUIRE(mq1.pushBack(data.c_str(), data.size()));
    REQUIRE(mq1.pushBack(data.c_str(), data.size()));
    REQUIRE(!mq1.pushBack(data.c_str(), data.size()));

    char buf10[10] = {0};
    int64_t written = 0L;
    REQUIRE(!mq1.popFront(buf10, 10, written));
    REQUIRE(written == 12);

    char buf20[20] = {0};
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(!mq1.popFront(buf20, 20, written));
    REQUIRE(written == 0);

    // again
    //
    REQUIRE(mq1.pushBack(data.c_str(), data.size()));
    REQUIRE(mq1.pushBack(data.c_str(), data.size()));
    REQUIRE(!mq1.pushBack(data.c_str(), data.size()));

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(!mq1.popFront(buf20, 20, written));
    REQUIRE(written == 0);

    mq1.close();
}

TEST_CASE("mq-push-pop-discard") {
    std::string mqPath = "mq-create-open-641EC1C27FE244C9B";

    veigar::MessageQueue mq1(true, 3, 10);  // max size = 30
    REQUIRE(mq1.create(mqPath));

    std::string data1 = "hello-123451";  // size = 12
    REQUIRE(mq1.pushBack(data1.c_str(), data1.size()));

    std::string data2 = "hello-123452";  // size = 12
    REQUIRE(mq1.pushBack(data2.c_str(), data2.size()));

    std::string data3 = "hello-123453";  // size = 12
    REQUIRE(mq1.pushBack(data3.c_str(), data3.size()));

    char buf10[10] = {0};
    int64_t written = 0L;
    REQUIRE(!mq1.popFront(buf10, 10, written));
    REQUIRE(written == 12);

    char buf20[20] = {0};
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data2);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data3);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(!mq1.popFront(buf20, 20, written));
    REQUIRE(written == 0);

    // again
    //
    REQUIRE(mq1.pushBack(data1.c_str(), data1.size()));
    REQUIRE(mq1.pushBack(data2.c_str(), data2.size()));
    REQUIRE(mq1.pushBack(data3.c_str(), data3.size()));

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data2);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(mq1.popFront(buf20, 20, written));
    REQUIRE(buf20 == data3);
    REQUIRE(written == 12);

    memset(&buf20[0], 0, 20);
    REQUIRE(!mq1.popFront(buf20, 20, written));
    REQUIRE(written == 0);

    mq1.close();
}

TEST_CASE("mq-multi-thread-push-pop") {
    std::string mqPath = "mq-multi-thread-push-pop-641EC1C27FE244C9B";

    veigar::MessageQueue mq(true, 10000, 20);  // max size = 30
    REQUIRE(mq.create(mqPath));

    int pushFailed = 0;
    int popFailed = 0;
    ThreadGroup tg;
    tg.createThreads(2, [&mq, &pushFailed, &popFailed](std::size_t id) {
        std::string data = "hello-123456";  // size = 12
        char buf20[20] = {0};
        for (int i = 0; i < 9999; i++) {
            if (id % 2 == 0) {
                if (!mq.pushBack(data.c_str(), data.size()))
                    pushFailed++;
            }
            else {
                if (i == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                int64_t written = 0;
                memset(&buf20[0], 0, 20);
                if (!mq.popFront(buf20, 20, written))
                    popFailed++;
            }
        }
    });

    tg.joinAll();
    mq.close();

    REQUIRE(pushFailed == 0);
    REQUIRE(popFailed == 0);
}
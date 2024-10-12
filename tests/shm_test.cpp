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
#include "catch.hpp"
#include "../src/shared_memory.h"
#include "thread_group.h"

TEST_CASE("shm-valid") {
    veigar::SharedMemory shm1("C519BBC0D2774887AB754A9463DBA664" + std::to_string(time(nullptr)), 1024);
    REQUIRE(!shm1.valid());
    REQUIRE(shm1.data() == nullptr);

    REQUIRE(shm1.create());
    REQUIRE(shm1.valid());
    REQUIRE(shm1.data() != nullptr);

    shm1.close();
    REQUIRE(!shm1.valid());
    REQUIRE(shm1.data() == nullptr);

    REQUIRE(shm1.create());

    REQUIRE(shm1.valid());
    REQUIRE(shm1.data() != nullptr);

    shm1.close();
}

TEST_CASE("shm-create-open-1") {
     std::string name = "C519BBC0D2774887AB754A9463DBA665" + std::to_string(time(nullptr));

    veigar::SharedMemory shm1(name, 1024);
    REQUIRE(!shm1.open());

    veigar::SharedMemory shm2(name, 1024);
    REQUIRE(shm2.create());

    veigar::SharedMemory shm3(name, 1024);
    REQUIRE(shm3.open());

    shm3.close();
    shm2.close();
    shm1.close();

    veigar::SharedMemory shm4(name, 1024);
    REQUIRE(!shm4.open());

    veigar::SharedMemory shm5(name, 1024);
    REQUIRE(shm5.create());

    veigar::SharedMemory shm6(name, 1024);
    REQUIRE(shm6.open());

    shm5.close();
    shm6.close();
}

TEST_CASE("shm-open2-close1-open") {
    std::string name = "C519BBC0D2774897AB754A9463DBA666" + std::to_string(time(nullptr));

    veigar::SharedMemory shm(name, 1024);
    REQUIRE(!shm.open());

    veigar::SharedMemory shm2(name, 1024);
    REQUIRE(shm2.create());
    REQUIRE(shm2.valid());

    veigar::SharedMemory shm3(name, 1024);
    REQUIRE(shm3.open());

    shm3.close();

    veigar::SharedMemory shm4(name, 1024);
    REQUIRE(shm4.open());

    shm2.close();
    shm4.close();

    veigar::SharedMemory shm5(name, 1024);
    REQUIRE(!shm5.open());
}

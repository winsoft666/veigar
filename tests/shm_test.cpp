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
    veigar::SharedMemory shm1("C519BBC0D2774887AB754A9463DBA664", 1024, true);
    REQUIRE(!shm1.valid());
    REQUIRE(shm1.data() == nullptr);

    REQUIRE(shm1.open());
    REQUIRE(shm1.valid());
    REQUIRE(shm1.data() != nullptr);

    shm1.close();
    REQUIRE(!shm1.valid());
    REQUIRE(shm1.data() == nullptr);

    REQUIRE(shm1.open());

    REQUIRE(shm1.valid());
    REQUIRE(shm1.data() != nullptr);

    shm1.close();
}

TEST_CASE("shm-create-open-1") {
    veigar::SharedMemory shm1("C519BBC0D2774887AB754A9463DBA665", 1024, false);
    REQUIRE(!shm1.open());

    veigar::SharedMemory shm2("C519BBC0D2774887AB754A9463DBA665", 1024, true);
    REQUIRE(shm2.open());

    veigar::SharedMemory shm3("C519BBC0D2774887AB754A9463DBA665", 1024, true);
    REQUIRE(shm3.open());

    shm3.close();
    shm2.close();
    shm1.close();

    veigar::SharedMemory shm4("C519BBC0D2774887AB754A9463DBA665", 1024, false);
    REQUIRE(!shm4.open());

    veigar::SharedMemory shm5("C519BBC0D2774887AB754A9463DBA665", 1024, true);
    REQUIRE(shm5.open());

    veigar::SharedMemory shm6("C519BBC0D2774887AB754A9463DBA665", 1024, false);
    REQUIRE(shm6.open());

    shm5.close();
    shm6.close();
}

TEST_CASE("shm-open2-close1-open") {
    veigar::SharedMemory shm("C519BBC0D2774897AB754A9463DBA666", 1024, false);
    REQUIRE(!shm.open());

    veigar::SharedMemory shm2("C519BBC0D2774897AB754A9463DBA666", 1024, true);
    REQUIRE(shm2.open());
    REQUIRE(shm2.valid());
    REQUIRE(shm2.open());

    veigar::SharedMemory shm3("C519BBC0D2774897AB754A9463DBA666", 1024, false);
    REQUIRE(shm3.open());

    shm3.close();

    veigar::SharedMemory shm4("C519BBC0D2774897AB754A9463DBA666", 1024, false);
    REQUIRE(shm4.open());

    shm2.close();
    shm4.close();

    veigar::SharedMemory shm5("C519BBC0D2774897AB754A9463DBA666", 1024, false);
    REQUIRE(!shm5.open());
}

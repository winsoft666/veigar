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
#include <map>
#include "veigar/veigar.h"
#include <vector>
#include "thread_group.h"
#include "catch.hpp"

std::string genRandomString(const uint32_t len) {
    srand(Catch::rngSeed());
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string ret;
    ret.reserve(len);

    for (uint32_t i = 0; i < len; ++i) {
        ret += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return ret;
}

TEST_CASE("init-unint") {
    const std::string channelName = "76B152577C2B4E8C89CE0E3D7029D570";
    veigar::Veigar vg;
    REQUIRE(vg.init(channelName));
    REQUIRE(vg.isInit());
    REQUIRE(vg.init(channelName));
    REQUIRE(vg.isInit());
    vg.uninit();
    REQUIRE(!vg.isInit());
    REQUIRE(vg.init(channelName));
    REQUIRE(vg.isInit());
    REQUIRE(!vg.init(channelName + "1"));
    REQUIRE(vg.isInit());
    vg.uninit();
    REQUIRE(!vg.isInit());
}

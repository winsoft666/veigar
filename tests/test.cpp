/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023-2024 winsoft666 <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <map>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "veigar/veigar.h"
#include <vector>
#include "thread_group.h"

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

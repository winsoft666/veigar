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
#include "catch.hpp"
#include "veigar/veigar.h"
#include <vector>

TEST_CASE("bind-unbind") {
    const std::string channelName = "6CEAEF1BE01B46128CC23BBED8605B3D";
    auto func1 = []() {};
    auto func2 = [](int a, bool b, std::string c) -> std::string { return "ok"; };
    auto func3 = [](int a, int b, int c) { return a + b + c; };
    auto func4 = [](std::string s) { return s.size(); };
    auto func5 = [](std::string s) { return s; };
    auto func6 = [](std::vector<uint8_t> buf1, std::vector<uint8_t> buf2) { return buf1.size(); };
    auto func7 = [](std::vector<uint8_t> buf) { return buf; };
    auto func8 = []() { return std::string("test"); };

    veigar::Veigar vg;

    REQUIRE(vg.bind("func1", func1));
    REQUIRE(!vg.bind("func1", func1));
    vg.unbind("func1");
    REQUIRE(vg.bind("func1", func1));

    REQUIRE(vg.bind("func2", func2));
    REQUIRE(!vg.bind("func2", func2));
    vg.unbind("func2");
    REQUIRE(vg.bind("func2", func2));

    REQUIRE(vg.bind("func3", func3));
    REQUIRE(!vg.bind("func3", func3));
    vg.unbind("func3");
    REQUIRE(vg.bind("func3", func3));

    REQUIRE(vg.bind("func4", func4));
    REQUIRE(!vg.bind("func4", func4));
    vg.unbind("func4");
    REQUIRE(vg.bind("func4", func4));

    REQUIRE(vg.bind("func5", func5));
    REQUIRE(!vg.bind("func5", func5));
    vg.unbind("func5");
    REQUIRE(vg.bind("func5", func5));

    REQUIRE(vg.bind("func6", func6));
    REQUIRE(!vg.bind("func6", func6));
    vg.unbind("func6");
    REQUIRE(vg.bind("func6", func6));

    REQUIRE(vg.bind("func7", func7));
    REQUIRE(!vg.bind("func7", func7));
    vg.unbind("func7");
    REQUIRE(vg.bind("func7", func7));

    REQUIRE(vg.bind("func8", func8));
    REQUIRE(!vg.bind("func8", func8));
    vg.unbind("func8");
    REQUIRE(vg.bind("func8", func8));

    REQUIRE(vg.init("test"));
    REQUIRE(vg.isInit());
    vg.uninit();
}

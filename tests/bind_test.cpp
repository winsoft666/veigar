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

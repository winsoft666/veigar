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
#include <queue>
#include <list>
#include <map>
#include <vector>
#if __cplusplus >= 201703
#include <string_view>
#include <optional>
#endif
#include "veigar/veigar.h"
#include "veigar/msgpack/adaptor/define.hpp"

TEST_CASE("type-test-1") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](char c, wchar_t w) {
        return w;
    }));
    CHECK(vg1.init("109F8B68-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", 'c', L'w');
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<wchar_t>() == L'w');

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("type-test-2") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](int i, unsigned int j, long k, long long l, unsigned long long m) {
        return i + j + k + l + m;
    }));
    CHECK(vg1.init("109F8B68-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", 1, 2, 3, 4, 5);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<int>() == 15);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("type-test-3") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](uint8_t i, int8_t j, uint32_t k, int32_t l, int64_t m, uint32_t n) {
        return i + j + k + l + m + n;
    }));
    CHECK(vg1.init("109F8B68-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", 1, 2, 3, 4, 5, 6);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<int>() == 21);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("type-test-4") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s) {
        return s + s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", "hello");
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<std::string>() == "hellohello");

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("type-test-5") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::vector<std::string> s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    std::vector<std::string> v = {"h", "i"};

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", v);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<std::vector<std::string>>() == v);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("type-test-6") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::map<std::string, int> s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    std::map<std::string, int> v = {{"h", 1}, {"i", 2}};

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", v);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<std::map<std::string, int>>() == v);

    vg1.uninit();
    vg2.uninit();
}

#if __cplusplus >= 201703
TEST_CASE("type-test-7") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string_view s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    std::string_view s = "hello";

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", s);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<std::string_view>() == s);

    vg1.uninit();
    vg2.uninit();
}
#endif

#if __cplusplus >= 201703
TEST_CASE("type-test-8") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::optional<std::string> s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    std::optional<std::string> opt;
    opt = "hi";

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", opt);
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<std::optional<std::string>>() == opt);

    vg1.uninit();
    vg2.uninit();
}
#endif

TEST_CASE("type-test-9") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::set<std::string> s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    std::set<std::string> v;
    v.emplace("h");
    v.emplace("i");

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", v);
    CHECK(cr.isSuccess());
    if (cr.isSuccess()) {
        CHECK(cr.obj.get().as<std::set<std::string>>() == v);
    }
    else {
        printf("ERROR: %d, %s\n", (int)cr.errCode, cr.errorMessage.c_str());
    }

    vg1.uninit();
    vg2.uninit();
}

struct MyPoint {
    int x;
    int y;
    MSGPACK_DEFINE(x, y);
};

TEST_CASE("type-test-10") {
    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](MyPoint s) {
        return s;
    }));
    CHECK(vg1.init("109F8B68-1"));

    MyPoint mp;
    mp.x = 10;
    mp.y = 11;

    veigar::Veigar vg2;
    CHECK(vg2.init("109F8B68-2"));
    veigar::CallResult cr = vg2.syncCall("109F8B68-1", 200, "func1", mp);
    CHECK(cr.isSuccess());
    if (cr.isSuccess()) {
        MyPoint mp2 = cr.obj.get().as<MyPoint>();
        CHECK((mp2.x == 10 && mp2.y == 11));
    }
    else {
        printf("ERROR: %d, %s\n", (int)cr.errCode, cr.errorMessage.c_str());
    }


    vg1.uninit();
    vg2.uninit();
}
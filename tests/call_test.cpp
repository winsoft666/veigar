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
#include <thread>
#include "catch.hpp"
#include "veigar/veigar.h"

TEST_CASE("call-sync-1") {
    std::string baseName = "call-sync-1-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-2"));
    veigar::CallResult cr = vg2.syncCall(baseName + "-1", 200, "func1", "s1", "s2");
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<int>() == 4);

    veigar::CallResult cr2 = vg2.syncCall(baseName + "-not-exist", 200, "func1", "s1", "s2");
    CHECK(!cr2.isSuccess());

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-sync-2") {
    std::string baseName = "call-sync-2-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-3"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-4"));

    veigar::CallResult cr = vg2.syncCall(baseName + "-3", 2000, "func1", "s1", "s2");
    CHECK(cr.isSuccess());
    CHECK(cr.obj.get().as<int>() == 4);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-sync-3") {
    std::string baseName = "call-sync-3-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-5"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-6"));

    veigar::CallResult cr = vg2.syncCall(baseName + "-3", 500, "func1", "s1", "s2");
    CHECK(!cr.isSuccess());

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-sync-recursion") {
    std::string baseName = "call-sync-recursion-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    vg1.bind("vg1-func", [baseName, &vg1](std::string s1, std::string s2) {
        veigar::CallResult cr1 = vg1.syncCall(baseName + "-8", 500, "vg2-func", "ss1", "ss2");
        CHECK(cr1.isSuccess());
        int result1 = 0;
        CHECK(cr1.convertObject(result1));
        CHECK(result1 == 6);

        return s1.size() + s2.size();
    });
    CHECK(vg1.init(baseName + "-7"));

    // ----

    veigar::Veigar vg2;
    vg2.bind("vg2-func", [](std::string s1, std::string s2) {
        return s1.size() + s2.size();
    });
    CHECK(vg2.init(baseName + "-8"));

    veigar::CallResult cr2 = vg2.syncCall(baseName + "-7", 500, "vg1-func", "s1", "s2");
    CHECK(cr2.isSuccess());
    if(cr2.isSuccess()) {
        int result2 = 0;
        CHECK(cr2.convertObject(result2));
        CHECK(result2 == 4);
    }
    else {
        printf("ERROR: %d, %s\n", (int)cr2.errCode, cr2.errorMessage.c_str());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-async-1") {
    std::string baseName = "call-async-1-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-1"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-2"));
    std::shared_ptr<veigar::AsyncCallResult> acr1 = vg2.asyncCall(baseName + "-1", 120, "func1", "s1", "s2");
    CHECK(acr1);
    CHECK(acr1->second.valid());
    CHECK(acr1->second.wait_for(std::chrono::milliseconds(300)) != std::future_status::timeout);
    CHECK(acr1->second.get().obj.get().as<int>() == 4);
    vg2.releaseCall(acr1->first);

    std::shared_ptr<veigar::AsyncCallResult> acr2 = vg2.asyncCall(baseName + "-not-exist", 120, "func1", "s1", "s2");
    CHECK(acr2);
    CHECK(acr2->second.valid());
    CHECK(acr2->second.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout);
    CHECK(!acr2->second.get().isSuccess());
    vg2.releaseCall(acr2->first);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-async-2") {
    std::string baseName = "call-async-2-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-3"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-4"));

    std::shared_ptr<veigar::AsyncCallResult> acr = vg2.asyncCall(baseName + "-3", 1200, "func1", "s1", "s2");
    CHECK(acr);
    CHECK(acr->second.valid());
    CHECK(acr->second.wait_for(std::chrono::milliseconds(2000)) != std::future_status::timeout);
    auto cr = acr->second.get();
    CHECK(cr.isSuccess());
    if(cr.isSuccess()) {
        CHECK(cr.obj.get().as<int>() == 4);
    }
    else {
        printf("ERROR: %d, %s\n", (int)cr.errCode, cr.errorMessage.c_str());
    }
    vg2.releaseCall(acr->first);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-async-3") {
    std::string baseName = "call-async-3-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    CHECK(vg1.bind("func1", [](std::string s1, std::string s2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return s1.size() + s2.size();
    }));
    CHECK(vg1.init(baseName + "-5"));

    veigar::Veigar vg2;
    CHECK(vg2.init(baseName + "-6"));

    std::shared_ptr<veigar::AsyncCallResult> acr = vg2.asyncCall(baseName + "-5", 120, "func1", "s1", "s2");
    CHECK(acr);
    CHECK(acr->second.valid());
    CHECK(acr->second.wait_for(std::chrono::milliseconds(500)) == std::future_status::timeout);
    vg2.releaseCall(acr->first);

    vg1.uninit();
    vg2.uninit();
}

TEST_CASE("call-async-recursion") {
    std::string baseName = "call-async-recursion-" + std::to_string(time(nullptr));

    veigar::Veigar vg1;
    vg1.bind("vg1-func", [baseName, &vg1](std::string s1, std::string s2) {
        std::shared_ptr<veigar::AsyncCallResult> acr1 = vg1.asyncCall(baseName + "-8", 500, "vg2-func", "ss1", "ss2");
        CHECK(acr1);
        CHECK(acr1->second.valid());
        CHECK(acr1->second.wait_for(std::chrono::milliseconds(500)) != std::future_status::timeout);
        auto cr = acr1->second.get();
        CHECK(cr.obj.get().as<int>() == 6);
        CHECK(cr.isSuccess());

        vg1.releaseCall(acr1->first);

        return s1.size() + s2.size();
    });
    CHECK(vg1.init(baseName + "-7"));

    // ----

    veigar::Veigar vg2;
    vg2.bind("vg2-func", [](std::string s1, std::string s2) {
        return s1.size() + s2.size();
    });
    CHECK(vg2.init(baseName + "-8"));

    std::shared_ptr<veigar::AsyncCallResult> acr2 = vg2.asyncCall(baseName + "-7", 500, "vg1-func", "s1", "s2");
    CHECK(acr2);
    CHECK(acr2->second.valid());
    CHECK(acr2->second.wait_for(std::chrono::milliseconds(500)) != std::future_status::timeout);
    auto cr = acr2->second.get();
    CHECK(cr.isSuccess());
    if(cr.isSuccess()) {
        CHECK(cr.obj.get().as<int>() == 4);
    }
    else {
        printf("ERROR: %d, %s\n", (int)cr.errCode, cr.errorMessage.c_str());
    }

    vg1.releaseCall(acr2->first);

    std::this_thread::sleep_for(std::chrono::milliseconds(8000));

    vg1.uninit();
    vg2.uninit();
}
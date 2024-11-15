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
#include "event.h"

using namespace veigar;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define IS_WINDOWS 1
#endif

#if IS_WINDOWS
#include <Windows.h>
#include <Shlwapi.h>
#include <strsafe.h>

#pragma comment(lib, "Shlwapi.lib")

bool CreateOtherProcess(const std::string& hostName, const std::string& otherName, const std::string& callFunc, HANDLE* pProcess) {
    char szDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, szDir, MAX_PATH);
    PathRemoveFileSpecA(szDir);
    PathAddBackslashA(szDir);

    char szFullCMD[1024];
#if (defined DEBUG) || (defined _DEBUG)
    StringCchPrintfA(szFullCMD, 1024, "\"%sother-d.exe\" %s %s %s", szDir, hostName.c_str(), otherName.c_str(), callFunc.c_str());
#else
    StringCchPrintfA(szFullCMD, 1024, "\"%sother.exe\" %s %s %s", szDir, hostName.c_str(), otherName.c_str(), callFunc.c_str());
#endif

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessA(NULL, szFullCMD, NULL, NULL, FALSE, 0, NULL, szDir, &si, &pi))
        return false;

    if (pi.hThread)
        CloseHandle(pi.hThread);

    if (pProcess) {
        *pProcess = pi.hProcess;
    }
    else {
        CloseHandle(pi.hProcess);
    }

    return true;
}
#else
// TODO
#endif

const std::string kOtherFuncName = "get_random_string";

TEST_CASE("outprocess-call-sync") {
    const std::string hostName = "outprocess-call-sync-host-" + std::to_string(time(nullptr));
    const std::string otherName = "outprocess-call-sync-other-" + std::to_string(time(nullptr));
    const std::string hostFuncName = "func1";

    Event otherProcessPrepared;
    Veigar v;
    REQUIRE(v.bind(hostFuncName, [&](std::string s1, std::string s2) {
        otherProcessPrepared.set();
        REQUIRE(s1.length() > 0);
        REQUIRE(s2.length() > 0);
        return s1 + s2;
    }));
    REQUIRE(v.init(hostName));

#if IS_WINDOWS
    HANDLE hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 100; i++) {
        CallResult cr = v.syncCall(otherName, 50, kOtherFuncName, i);
        CHECK(cr.isSuccess());
        std::string str;

        CHECK_NOTHROW((str = cr.obj.get().as<std::string>()));

        CHECK(str.length() == i);

        CallResult cr2 = v.syncCall(otherName + "-not-exist", 50, kOtherFuncName, 100);
        CHECK(!cr2.isSuccess());

        CallResult cr3 = v.syncCall(otherName, 50, kOtherFuncName + "-not-exist", 100);
        CHECK(!cr3.isSuccess());
        CHECK(!cr3.errorMessage.empty());
    }

#if IS_WINDOWS
    WaitForSingleObject(hProcess, 5000);

    DWORD dwExitCode = 0;
    GetExitCodeProcess(hProcess, &dwExitCode);
    CHECK(dwExitCode == 0);
#else
    // TODO
#endif
    v.uninit();
}

TEST_CASE("outprocess-call-async-1") {
    const std::string hostName = "outprocess-call-async-1-host-" + std::to_string(time(nullptr));
    const std::string otherName = "outprocess-call-async-1-other-" + std::to_string(time(nullptr));
    const std::string hostFuncName = "func111";

    Event otherProcessPrepared;
    Veigar v;
    REQUIRE(v.bind(hostFuncName, [&](std::string s1, std::string s2) {
        otherProcessPrepared.set();
        REQUIRE(s1.length() > 0);
        REQUIRE(s2.length() > 0);
        return s1 + s2;
    }));
    REQUIRE(v.init(hostName));

#if IS_WINDOWS
    HANDLE hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 100; i++) {
        {
            std::shared_ptr<AsyncCallResult> acr = v.asyncCall(otherName, 50, kOtherFuncName, i);
            CHECK(acr);
            CHECK(acr->second.valid());
            std::future_status waitResult = acr->second.wait_for(std::chrono::milliseconds(50));
            CHECK(waitResult != std::future_status::timeout);

            CallResult cr = std::move(acr->second.get());
            CHECK(cr.isSuccess());
            std::string str;

            CHECK_NOTHROW((str = cr.obj.get().as<std::string>()));

            CHECK(str.length() == i);
        }

        // not exists
        {
            std::shared_ptr<AsyncCallResult> acr2 = v.asyncCall(otherName, 50, kOtherFuncName + "-not-exists", i);
            CHECK(acr2);
            CHECK(acr2->second.valid());
            std::future_status waitResult2 = acr2->second.wait_for(std::chrono::milliseconds(50));
            CHECK(waitResult2 != std::future_status::timeout);

            CallResult cr2 = std::move(acr2->second.get());
            CHECK(!cr2.isSuccess());
            CHECK(!cr2.errorMessage.empty());
        }
    }

#if IS_WINDOWS
    WaitForSingleObject(hProcess, 5000);

    DWORD dwExitCode = 0;
    GetExitCodeProcess(hProcess, &dwExitCode);
    CHECK(dwExitCode == 0);
#else
    // TODO
#endif
    v.uninit();
}

TEST_CASE("outprocess-call-sync-kill-call-sync") {
    const std::string hostName = "outprocess-call-sync-kill-call-sync-host-" + std::to_string(time(nullptr));
    const std::string otherName = "outprocess-call-sync-kill-call-sync-other-" + std::to_string(time(nullptr));
    const std::string hostFuncName = "func123";

    Event otherProcessPrepared;
    Veigar v;
    REQUIRE(v.bind(hostFuncName, [&](std::string s1, std::string s2) {
        otherProcessPrepared.set();
        REQUIRE(s1.length() > 0);
        REQUIRE(s2.length() > 0);
        return s1 + s2;
    }));
    REQUIRE(v.init(hostName));

#if IS_WINDOWS
    HANDLE hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 100; i++) {
        CallResult cr = v.syncCall(otherName, 50, kOtherFuncName, i);
        CHECK(cr.isSuccess());
        std::string str;

        CHECK_NOTHROW((str = cr.obj.get().as<std::string>()));

        CHECK(str.length() == i);

        CallResult cr2 = v.syncCall(otherName + "-not-exist", 50, kOtherFuncName, 100);
        CHECK(!cr2.isSuccess());

        CallResult cr3 = v.syncCall(otherName, 50, kOtherFuncName + "-not-exist", 100);
        CHECK(!cr3.isSuccess());
        CHECK(!cr3.errorMessage.empty());
    }

#if IS_WINDOWS
    TerminateProcess(hProcess, 0);
#endif

    otherProcessPrepared.reset();

#if IS_WINDOWS
    hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 100; i++) {
        CallResult cr = v.syncCall(otherName, 50, kOtherFuncName, i);
        CHECK(cr.isSuccess());
        std::string str;

        CHECK_NOTHROW((str = cr.obj.get().as<std::string>()));

        CHECK(str.length() == i);

        CallResult cr2 = v.syncCall(otherName + "-not-exist", 50, kOtherFuncName, 100);
        CHECK(!cr2.isSuccess());

        CallResult cr3 = v.syncCall(otherName, 50, kOtherFuncName + "-not-exist", 100);
        CHECK(!cr3.isSuccess());
        CHECK(!cr3.errorMessage.empty());
    }

#if IS_WINDOWS

    WaitForSingleObject(hProcess, 5000);

    DWORD dwExitCode = 0;
    GetExitCodeProcess(hProcess, &dwExitCode);
    CHECK(dwExitCode == 0);
#else
    // TODO
#endif
    v.uninit();
}

TEST_CASE("outprocess-call-async-kill-call-async") {
    const std::string hostName = "outprocess-call-async-kill-call-async-host-" + std::to_string(time(nullptr));
    const std::string otherName = "outprocess-call-async-kill-call-async-other-" + std::to_string(time(nullptr));
    const std::string hostFuncName = "func111";

    Event otherProcessPrepared;
    Veigar v;
    REQUIRE(v.bind(hostFuncName, [&](std::string s1, std::string s2) {
        otherProcessPrepared.set();
        REQUIRE(s1.length() > 0);
        REQUIRE(s2.length() > 0);
        return s1 + s2;
    }));
    REQUIRE(v.init(hostName, 200, 10240));

#if IS_WINDOWS
    HANDLE hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 180; i++) {
        std::shared_ptr<AsyncCallResult> acr = v.asyncCall(otherName, 50, kOtherFuncName, i);
    }

#if IS_WINDOWS
    TerminateProcess(hProcess, 0);
#endif

    otherProcessPrepared.reset();

#if IS_WINDOWS
    hProcess = NULL;
    REQUIRE(CreateOtherProcess(hostName, otherName, hostFuncName, &hProcess));
#else
    // TODO
#endif

    REQUIRE(otherProcessPrepared.wait(500));

    for (int i = 0; i < 100; i++) {
        {
            std::shared_ptr<AsyncCallResult> acr = v.asyncCall(otherName, 50, kOtherFuncName, i);
            CHECK(acr);
            CHECK(acr->second.valid());
            std::future_status waitResult = acr->second.wait_for(std::chrono::milliseconds(50));
            CHECK(waitResult != std::future_status::timeout);

            CallResult cr = std::move(acr->second.get());
            CHECK(cr.isSuccess());
            std::string str;

            CHECK_NOTHROW((str = cr.obj.get().as<std::string>()));

            CHECK(str.length() == i);
        }

        // not exists
        {
            std::shared_ptr<AsyncCallResult> acr2 = v.asyncCall(otherName, 50, kOtherFuncName + "-not-exists", i);
            CHECK(acr2);
            CHECK(acr2->second.valid());
            std::future_status waitResult2 = acr2->second.wait_for(std::chrono::milliseconds(50));
            CHECK(waitResult2 != std::future_status::timeout);

            CallResult cr2 = std::move(acr2->second.get());
            CHECK(!cr2.isSuccess());
            CHECK(!cr2.errorMessage.empty());
        }
    }

#if IS_WINDOWS
    WaitForSingleObject(hProcess, 5000);

    DWORD dwExitCode = 0;
    GetExitCodeProcess(hProcess, &dwExitCode);
    CHECK(dwExitCode == 0);
#else
    // TODO
#endif
    v.uninit();
}

/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "veigar/veigar.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <atomic>
#include <cstdlib>  //std::system
#include <inttypes.h>

using namespace veigar;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define IS_WINDOWS 1
#endif

#if IS_WINDOWS
bool IsWow64(HANDLE process, bool& result) {
    BOOL bIsWow64 = FALSE;

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process =
        (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (NULL == fnIsWow64Process) {
        return false;
    }

    if (!fnIsWow64Process(process, &bIsWow64)) {
        return false;
    }

    result = !!bIsWow64;
    return true;
}

bool IsWin64(HANDLE process) {
#if (defined _WIN64) || (defined WIN64)
    return true;
#else
    bool result = false;
    IsWow64(process, result);
    return result;
#endif
}

bool Is32BitProcess() {
    HANDLE process = GetCurrentProcess();
    if (!process)
        return false;

    bool wow64 = false;
    IsWow64(process, wow64);
    if (wow64)
        return true;

    BOOL win64 = IsWin64(process);
    return !win64;
}

#endif

std::string TimeToHuman(int64_t microseconds) {
    int64_t hour = microseconds / 3600000000;
    int64_t min = (microseconds % 3600000000) / 60000000;
    int64_t sec = (microseconds % 60000000) / 1000000;
    int64_t mill = (microseconds % 1000000) / 1000;
    int64_t micro = microseconds % 1000;

    std::string str;
    if (hour > 0)
        str += std::to_string(hour) + "h";
    if (min > 0)
        str += std::to_string(min) + "m";
    if (sec > 0)
        str += std::to_string(sec) + "s";
    if (mill > 0)
        str += std::to_string(mill) + "ms";
    if (micro > 0)
        str += std::to_string(micro) + "μs";
    return str;
}

std::string genRandomString(const uint32_t len) {
    srand((uint32_t)time(nullptr) + len);
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

int result = 999;

// other.exe hostChannelName selfChannelName callFunc
//
int main(int argc, char** argv) {
    if (argc < 3)
        return 1;

    std::string hostChannelName = argv[1];
    if (hostChannelName.empty())
        return 2;

    std::string selfChannelName = argv[2];
    if (selfChannelName.empty())
        return 2;

    Veigar vg;
    if (!vg.init(selfChannelName)) {
        return 3;
    }

    bool ret = vg.bind("get_random_string", [](int64_t len) {
        std::string str = genRandomString(len);
        return str;
    });

    if (!ret) {
        vg.uninit();
        return 4;
    }

    if (argc >= 4) {
        std::string func = argv[3];
        if (!func.empty()) {
            try {
                std::string randStr1 = genRandomString(10);
                std::string randStr2 = genRandomString(20);
                std::shared_ptr<AsyncCallResult> r = vg.asyncCall(hostChannelName, 50, func, randStr1, randStr2);
                assert(r);
                if (r) {
                    if (r->second.valid()) {
                        CallResult cr;
                        auto waitResult = r->second.wait_for(std::chrono::milliseconds(50));
                        if (waitResult == std::future_status::timeout) {
                            result = 5;
                        }
                        else {
                            cr = std::move(r->second.get());
                            if (cr.isSuccess()) {
                                if (cr.obj.get().as<std::string>() == randStr1 + randStr2) {
                                    result = 0;
                                }
                                else {
                                    result = 6;
                                }
                            }
                            else {
                                result = 7;
                            }
                        }
                    }
                    vg.releaseCall(r->first);
                }
            }
            catch (...) {
                result = 8;
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    vg.uninit();

    return result;
}

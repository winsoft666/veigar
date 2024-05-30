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
#include "thread_group.h"

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
        str += std::to_string(micro) + "¦Ìs";
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

veigar::Veigar vg;
std::string channelName;
std::string targetChannel;
std::string strPayload;
const int threadNum = 4;
const int64_t eachThreadCallNum = 25000;
std::atomic<int64_t> success = 0;
std::atomic<int64_t> timeout = 0;
std::atomic<int64_t> failed = 0;

void CallThreadProc(std::size_t threadId, std::string targetChannel) {
    for (int i = 0; i < eachThreadCallNum; i++) {
        veigar::CallResult cr = vg.syncCall(targetChannel, 1000, "echo", strPayload);
        if (cr.isSuccess() && cr.obj.get().as<std::string>() == strPayload) {
            success++;
        }
        else {
            if (cr.errCode == veigar::ErrorCode::TIMEOUT) {
                timeout++;
            }
            else {
                failed++;
            }
            printf("%s\n", cr.errorMessage.c_str());
        }
    }
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    printf("Veigar: Cross platform RPC library using shared memory.\n");
#if IS_WINDOWS
    printf("Version: %d.%d (%s)\n", veigar::VERSION_MAJOR, veigar::VERSION_MINOR, Is32BitProcess() ? "x86" : "x64");
#else
    printf("Version: %d.%d\n", veigar::VERSION_MAJOR, veigar::VERSION_MINOR);
#endif
    printf("Input 'quit' to exit the program.\n");
    printf("\n");

    vg.setTimeoutOfRWLock(1000);

    while (true) {
        if (channelName.empty()) {
            std::cout << "Channel Name: ";
            std::cin >> channelName;

            if (channelName.empty()) {
                std::cout << "Channel name can not be empty.\n";
                continue;
            }

            if (channelName == "quit")
                break;

            if (!vg.init(channelName)) {
                printf("Init failed.\n");
                channelName.clear();
                continue;
            }

            std::cout << "Init success.\n";

            if (!vg.bind("echo", [](const std::string& msg) {
                    return msg;
                })) {
                printf("Bind echo function failed.\n");
                continue;
            }

            std::cout << "Bind functions success.\n";
            std::cout << "Armed.\n\n";
        }
        else {
            std::cout << "Target channel names: ";
            std::cin >> targetChannel;

            if (targetChannel == "quit")
                break;

            int payloadSize = 0;
            std::cout << "Payload size: ";
            std::cin >> payloadSize;
            if (payloadSize <= 0) {
                std::cout << "Payload size must greater than 0.\n";
                continue;
            }

            strPayload = genRandomString(payloadSize);

            success.store(0);
            timeout.store(0);
            failed.store(0);

            auto tg = std::make_shared<ThreadGroup>();
            veigar::detail::TimeMeter tm;
            tg->createThreads(threadNum, targetChannel, CallThreadProc);
            tg->joinAll();

            const int64_t totalCall = eachThreadCallNum * threadNum;

            assert(success.load() + failed.load() + timeout.load() == totalCall);
            if (success.load() + failed.load() + timeout.load() == totalCall) {
                int64_t used = tm.elapsed();
                printf("Used: %s, Total: %" PRId64 " Success: %" PRId64 ", Timeout: %" PRId64 ", Failed: %" PRId64 ", Average: %s/call.\n",
                       TimeToHuman(used).c_str(), totalCall, success.load(), timeout.load(), failed.load(), TimeToHuman(used / totalCall).c_str());
            }
        }
    }

    if (vg.isInit()) {
        vg.uninit();
    }

    return 0;
}

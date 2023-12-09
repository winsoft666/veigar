/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023 winsoft666 <winsoft666@outlook.com>.
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
#include "veigar/veigar.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <mutex>
#include <cstdlib>  //std::system
#include <inttypes.h>
#include "thread_group.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define IS_WINDOWS 1
#endif

#if IS_WINDOWS
bool IsWow64(HANDLE process, bool& result) noexcept {
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

bool IsWin64(HANDLE process) noexcept {
#if (defined _WIN64) || (defined WIN64)
    return true;
#else
    bool result = false;
    IsWow64(process, result);
    return result;
#endif
}

bool Is32BitProcess() noexcept {
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

std::string str1046 = R"(
no4vvZcIIRoc3xz9ZkccRYo9XiliFGQqhnAk5zKz85ttbSI8IcOUGUx3OXagffEzSbuAO6W930KRSFBY0P6JlNPB0QyhhsFYl
vSHkwDnZOfLdWsS2zmDBa8It5Wcw9LWmKejilmpguZDO8vmtg4N1hUu2ayLJvoDAsQHiT5xL9qkF6KEznEba1ktT3grRNoGeZ
uESEHgdgxkViuLGXhkQX0wDVgGnMO9xsrWHtUu4WyXCPRXF7XGIvLaIRce1uyAKEb5SzeIg7A4fLXzufvdo8AholKI5nwQ0xh
1tnfOYtLLzvElGlkyJjHRoviSi1StkQKwKjHWIFYyVASDKzSTPWuqPFNXOigKsMlWkTerzSR6Ms1W9JBMxY8DB2XVMm7Nd1R7
vuoPEKJVwW7kIhCT6UQGG24s9GcQNayaM3BWg64TqGLhoYODkGgRJRwl0T1Iwg5opi8qyvKvW1B4oFL3Y1BbUSpsWsTXdt7OQ
qJqlTFrgKUsu5i5SYZOQezyU9YbrkcOvcrrEJFohV3yBoLRftslq7OvzgEhSFhTQRlXb0HwM30xoBgLdG44PLZ7N6pRqrAxmp
PK3WURadvOQKr4b6xPsLD5FA7jwRx8a8ZYIKece9DYgQ6V4YllSXiPytCmeNcvjnQK7pCLYiB6HUpcnT6QuH3FhU6APSOqN10
7krgZYdb1ucYRnLVz0PKLLpGGfw5RPDkhxkIKDO2bmFS4OwoSVip0ZWSKwYiM2xWgWFsCLE0y8ppC7kK3ixwdouss9Rvq9Y3W
I5uuTVNcilebnLkPwmlCIUclNETCVyxyXR0CUuRIHO0bXd6S0rxFSyej4TGo4UEecNxuTCsA6Ub9fgMwloKwYJKomiO83xmws
X7RHvEcSqriLCNS4etwZawJaQWiyVud9PZL4Ixxg7HuRBMwJtlYn7bx2Yx96RmItUU8DjtGZVUrbnTcLzeAGyqGhRjfzHTLwC
gVsPnzU5oqkOD3aNUCEs74gxRWQGR0iv9A17Fsnwgd44UBxOnAIlFXaNLZyn3reVSQJQnWpag8Wa
)";

const uint32_t warnDelayMicroseconds = 500000;  // 500ms

std::vector<std::string> StringSplit(const std::string& src, const std::string& delimiter, bool includeEmptyStr) {
    std::vector<std::string> fields;
    typename std::string::size_type offset = 0;
    typename std::string::size_type pos = src.find(delimiter, 0);

    while (pos != std::string::npos) {
        std::string t = src.substr(offset, pos - offset);
        if ((t.length() > 0) || (t.length() == 0 && includeEmptyStr))
            fields.push_back(t);
        offset = pos + delimiter.length();
        pos = src.find(delimiter, offset);
    }

    const std::string t = src.substr(offset);
    if ((t.length() > 0) || (t.length() == 0 && includeEmptyStr))
        fields.push_back(t);
    return fields;
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

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    printf("Veigar: Cross platform RPC library using shared memory.\n");
#if IS_WINDOWS
    printf("Version: %d.%d (%s)\n\n", veigar::VERSION_MAJOR, veigar::VERSION_MINOR, Is32BitProcess() ? "x86" : "x64");
#else
    printf("Version: %d.%d\n\n", veigar::VERSION_MAJOR, veigar::VERSION_MINOR);
#endif

    std::string channelName;
    int outputRecv = 0;

    veigar::Veigar vg;

    while (true) {
        if (channelName.empty()) {
            std::cout << "Channel Name:";
            std::cin >> channelName;

            if (channelName.empty()) {
                std::cout << "Channel name can not be empty,\n";
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

            if (!vg.bind("echo", [&outputRecv](const std::string& msg, int id) {
                    if (outputRecv != 0) {
                        std::cout << "RECV:" << msg << std::endl;
                    }
                    return msg + "_" + std::to_string(id);
                })) {
                printf("Bind echo function failed.\n");
                continue;
            }

            std::cout << "Bind functions success.\n";
            std::cout << "Output received message(0/1): ";
            std::cin >> outputRecv;
            std::cout << "Armed.\n\n";
        }
        else {
            int asyncMethod = 0;
            int threadNum = 0;
            int callTimesEachThread = 0;
            int rwTimeout = 0;
            std::string targetChannels;
            std::vector<std::string> targetChannelList;

            std::cout << "Target channel names (Split by comma):\n";
            std::cin >> targetChannels;

            if (targetChannels == "quit")
                break;

            targetChannelList = StringSplit(targetChannels, ",", false);
            if (targetChannelList.empty())
                continue;

            std::cout << "Async method(0/1): \n";
            std::cin >> asyncMethod;

            std::cout << "Thread number for each target: \n";
            std::cin >> threadNum;

            std::cout << "Call times each of thread: \n";
            std::cin >> callTimesEachThread;

            std::cout << "Read/Write Timeout(ms):\n";
            std::cin >> rwTimeout;

            vg.setReadWriteTimeout(rwTimeout);

            auto fn = [&vg, rwTimeout, channelName, asyncMethod, callTimesEachThread](std::size_t threadId, std::string targetChannel) {
                int error = 0;
                int success = 0;
                int64_t totalUsed = 0;

                printf("[Thread %" PRId64 ", Target %s] Calling...\n", (int64_t)threadId, targetChannel.c_str());
                
                std::string msgPre = channelName + "_" + targetChannel + "_" + std::to_string(threadId) + "_";
                for (int i = 0; i < callTimesEachThread; i++) {
                    std::string msg = msgPre + std::to_string(i);

                    veigar::detail::TimeMeter tm;
                    veigar::CallResult ret;

                    if (asyncMethod == 0) {
                        ret = vg.syncCall(targetChannel, rwTimeout * 3, "echo", msg, i);
                    }
                    else {
                        std::shared_ptr<veigar::AsyncCallResult> acr = vg.asyncCall(targetChannel, "echo", msg, i);
                        assert(acr);
                        if (acr) {
                            if (acr->second.valid()) {
                                auto waitResult = acr->second.wait_for(std::chrono::milliseconds(rwTimeout * 3));
                                if (waitResult == std::future_status::timeout) {
                                    ret.errCode = veigar::ErrorCode::TIMEOUT;
                                    ret.errorMessage = "Timeout";
                                }
                                else {
                                    ret = std::move(acr->second.get());
                                }
                            }
                            vg.releaseCall(acr->first);
                        }
                    }

                    int64_t used = tm.elapsed();
                    totalUsed += used;

                    std::string expectResultStr = msg + "_" + std::to_string(i);
                    if (ret.isSuccess() && ret.obj.get().as<std::string>() == expectResultStr) {
                        success++;
                        if (used >= warnDelayMicroseconds) {
                            printf("[Thread %" PRId64 ", Target %s] Warning: call %d take long time: %" PRId64 "us >= %dus\n",
                                   (int64_t)threadId, targetChannel.c_str(), i, tm.elapsed(), warnDelayMicroseconds);
                        }
                    }
                    else {
                        error++;
                        std::cout << ret.errorMessage << std::endl;
                    }
                }

                printf("[Thread %" PRId64 ", Target %s] Total %d, Success %d, Error %d, Used: %" PRId64 "us, Average: %" PRId64 "us/call, %" PRId64 "call/s.\n\n",
                       (int64_t)threadId, targetChannel.c_str(),
                       callTimesEachThread, success, error, totalUsed,
                       (int64_t)((double)totalUsed / (double)callTimesEachThread),
                       (int64_t)(1000000.0 / ((double)totalUsed / (double)callTimesEachThread)));
            };

            std::vector<std::shared_ptr<ThreadGroup>> threadGroups;

            for (auto targetChannel : targetChannelList) {
                auto tg = std::make_shared<ThreadGroup>();
                tg->createThreads(threadNum, targetChannel, fn);
                threadGroups.push_back(tg);
            }

            for (auto tg : threadGroups) {
                tg->joinAll();
            }
            threadGroups.clear();
        }
    }

    if (vg.isInit()) {
        vg.uninit();
    }

    return 0;
}

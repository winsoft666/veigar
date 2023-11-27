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
#include "thread_group.h"

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

const unsigned int warnDelayMS = 200;

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
    srand((unsigned int)time(nullptr) + len);
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
    printf("Veigar: Cross platform RPC library using shared memory.\n\n");

    std::string channelName;
    veigar::Veigar vg;
    int outputRecv = 0;

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

            auto fn = [&vg, channelName, asyncMethod, callTimesEachThread](std::size_t threadId, std::string targetChannel) {
                int error = 0;
                int success = 0;

                printf("[Thread %lld, Target %s] Calling...\n", (int64_t)threadId, targetChannel.c_str());
                veigar::detail::TimeMeter threadTM;
                for (int i = 0; i < callTimesEachThread; i++) {
                    std::string msg = str1046 + channelName + "_" + targetChannel + "_" + std::to_string(i) + "_" + std::to_string(threadId);
                    veigar::detail::TimeMeter tm;
                    veigar::CallResult ret;

                    if (asyncMethod == 0) {
                        ret = vg.syncCall(targetChannel, warnDelayMS * 2, "echo", msg, i);
                    }
                    else {
                        std::shared_ptr<veigar::AsyncCallResult> acr = vg.asyncCall(targetChannel, "echo", msg, i);
                        if (acr->second.valid()) {
                            auto waitResult = acr->second.wait_for(std::chrono::milliseconds(warnDelayMS * 2));
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

                    long used = tm.Elapsed();
                    if (used >= warnDelayMS) {
                        printf("[Thread %lld, Target %s] Warn: call %d take long time: %dms >= %dms\n",
                               (int64_t)threadId, targetChannel.c_str(), i, tm.Elapsed(), warnDelayMS);
                    }

                    std::string expectResultStr = msg + "_" + std::to_string(i);

                    if (ret.isSuccess() && ret.obj.get().as<std::string>() == expectResultStr) {
                        success++;
                    }
                    else {
                        error++;
                        std::cout << ret.errorMessage << std::endl;
                    }
                }
                printf("[Thread %lld, Target %s] Total %d, Success %d, Error %d, Used: %dms.\n\n",
                       (int64_t)threadId, targetChannel.c_str(),
                       callTimesEachThread, success, error, threadTM.Elapsed());
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

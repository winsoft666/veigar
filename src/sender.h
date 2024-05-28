#ifndef VEIGAR_SENDER_H_
#define VEIGAR_SENDER_H_
#pragma once

#include <queue>
#include <mutex>
#include <vector>
#include <string>
#include "event.h"
#include "resp_dispatcher.h"
#include "message_queue.h"

namespace veigar {
class Veigar;
class Sender {
   public:
    struct CallMeta {
        std::string channel;
        std::string callId;
        ResultMeta resultMeta;
        uint8_t* data = nullptr;
        size_t dataSize = 0;
        int64_t startCallTimePoint;  // microseconds
        int64_t timeout = 0;         // microseconds
    };
    struct RespMeta {
        std::string channel;
        uint8_t* data = nullptr;
        size_t dataSize = 0;
        int64_t startCallTimePoint;  // microseconds
        int64_t timeout = 0;         // microseconds
    };
    Sender(Veigar* v) noexcept;
    ~Sender() noexcept = default;

    bool init(std::shared_ptr<RespDispatcher> respDisp,
              std::shared_ptr<MessageQueue> selfCallMQ,
              std::shared_ptr<MessageQueue> selfRespMQ);

    void uninit();

    bool isInit() const;

    void addCall(const Sender::CallMeta& cm);
    void addResp(const Sender::RespMeta& rm);

   private:
    std::shared_ptr<MessageQueue> getTargetCallMessageQueue(const std::string& channelName);
    std::shared_ptr<MessageQueue> getTargetRespMessageQueue(const std::string& channelName);

    void callSendThreadProc();
    void respSendThreadProc();

    bool checkSpaceAndWait(std::shared_ptr<MessageQueue> mq,
                           int64_t needSize,
                           int64_t startCallTimePoint,
                           int64_t timeout);

   private:
    bool isInit_ = false;
    Event stopEvent_;

    Veigar* veigar_ = nullptr;

    std::shared_ptr<RespDispatcher> respDisp_ = nullptr;
    std::shared_ptr<MessageQueue> selfCallMQ_ = nullptr;
    std::shared_ptr<MessageQueue> selfRespMQ_ = nullptr;

    std::mutex callListMutex_;
    std::queue<CallMeta> callList_;
    std::vector<std::thread> callWorkers_;

    std::mutex respListMutex_;
    std::queue<RespMeta> respList_;
    std::vector<std::thread> respWorkers_;

    std::mutex targetCallMQsMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> targetCallMsgQueues_;

    std::mutex targetRespMQsMutex_;
    std::unordered_map<std::string, std::shared_ptr<MessageQueue>> targetRespMsgQueues_;
};
}  // namespace veigar

#endif  // !VEIGAR_SENDER_H_
